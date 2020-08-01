//   author : "Shiyue Li"
//   student ID : 17341075
//   class : AI & BIG DATA
//   finish date : 2020/08/01 Saturday



#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>


using namespace std;

// syntax tree
struct SyntaxNode{
	char chr;
	int pos;
	vector<int> firstpos;
	vector<int> lastpos;

	SyntaxNode *left;
	SyntaxNode *right;
	SyntaxNode(char c=' ', int p=-1){
		chr = c;
		pos = p;	// if p == -1 node: chr is operator
		left = NULL;
		right = NULL;
	}
};


class DFA{	
	private:
		vector<vector<int> > Dstates;		// store states (index of dim-0 as marker) 
		vector<vector<int> > Dtrans;		// store transition rules ; size = num_states * num_symbols
		vector<int> end_states;				// store the markers of end states
		vector<char> symbols;				// symbol set
		int num_states;						// number of states in dfa 
	
	public:
		DFA(vector<vector<int> > & s, vector<vector<int> > & trans, vector<int> & es, vector<char> & sym, int num_states){
			this->num_states = num_states;
			for(int i=0;i<this->num_states;i++){
				int size = s[i].size();
				this->Dstates.push_back(vector<int>(size, 0));
				for(int j=0;j<size;j++){
					this->Dstates[i][j] = s[i][j];
				}
				int num_symbols = sym.size();
				this->Dtrans.push_back(vector<int>(num_symbols, -1));		// -1 as empty
				for(int k=0;k<num_symbols;k++){
					this->Dtrans[i][k] = trans[i][k];
				}
			}
			this->end_states.assign(es.begin(), es.end());
			this->symbols.assign(sym.begin(), sym.end());
		}
		DFA(){
			this->num_states = 0;
		}
		DFA(const DFA &obj){
			this->num_states = obj.num_states;
			for(int i=0;i<this->num_states;i++){
				this->Dstates.push_back(obj.Dstates[i]);
			}
			this->end_states.assign(obj.end_states.begin(), obj.end_states.end());	
			this->symbols.assign(obj.symbols.begin(), obj.symbols.end());

			int num_symbols = this->symbols.size();

			for(int i=0;i<this->num_states;i++){
				this->Dtrans.push_back(vector<int>(num_symbols, -1));
				for(int j=0;j<num_symbols;j++){
					this->Dtrans[i][j] = obj.Dtrans[i][j];
				}
			}
			
		}
		
		int getNumStates(){
			return this->num_states;
		}
		int getNumSymbols(){
			return this->symbols.size();
		}
		int getNextStateName(int s, int chr){
			return this->Dtrans[s][chr];
		}
		vector<int> getState(int i){
			return vector<int>(this->Dstates[i]);
		}
		vector<char> getSymbols(){
			return vector<char>(this->symbols);
		}
		vector<int> getEndStates(){
			return vector<int>(this->end_states);
		}
		// get all the states that of non-accepted states
		void reverseEnd(vector<int> & reverse){
			for(int i=0;i<this->num_states;i++){
				bool flag = false;
				for(int j=0;j<this->end_states.size();j++){
					if(i==this->end_states[j]){
						flag = true;
						break;
					}
				}
				if(!flag){							// non-accepted state
					reverse.push_back(i);
				}
			}
			
		}
		
};

// calculate the union of two lists
vector<int> calUnion(vector<int> & v1, vector<int> & v2){
	set<int> help;
	vector<int> result;
	for(int i=0;i<v1.size();i++){
		help.insert(v1[i]);
	}
	for(int j=0;j<v2.size();j++){
		help.insert(v2[j]);
	}
	if(!help.empty()){
		result.assign(help.begin(), help.end());
		sort(result.begin(), result.end());								// need or not??
	}
	return result;
}

// judge whether the given node of syntax tree is nullable
bool nullable(SyntaxNode * node){
	if(node->chr == 'E'){											// a leaf labeled E
		return true;
	}
	if(node->chr == '+'){
		return nullable(node->right);
	}
	if(node->chr == '?'){
		return true;
	}
	if(node->pos > 0){												// a leaf with position i
		return false;
	}
	if(node->chr == '-'){											// a cat-ndoe
		return nullable(node->left) && nullable(node->right);
	}
	if(node->chr == '|'){											// a or-ndoe
		return nullable(node->left) || nullable(node->right);
	}
	if(node->chr == '*'){											// closure
		return true;
	}
	
	return false;
}

// calculate the firstpos set of a given node
void calFirstpos(SyntaxNode * node){
	
	// if(node->chr == 'E'){ do nothing }			// a leaf labeled empty string E
		
	if(node->pos > 0){								// a leaf with position i
		node->firstpos.push_back(node->pos);
	}
	if(node->chr == '|'){							// an or-node node = c1 | c2
		vector<int> temp(calUnion(node->left->firstpos, node->right->firstpos));
		node->firstpos.swap(temp);
	}
	if(node->chr == '-'){							// a cat-node n = c1c2
		if(nullable(node->left)){
			vector<int> temp(calUnion(node->left->firstpos, node->right->firstpos));
			node->firstpos.swap(temp);
		}
		else{
			node->firstpos.assign(node->left->firstpos.begin(), node->left->firstpos.end());
		}
	}
	if(node->chr == '+'){
		node->firstpos.assign(node->right->firstpos.begin(), node->right->firstpos.end());
	}
	if(node->chr == '?'){
		node->firstpos.assign(node->right->firstpos.begin(), node->right->firstpos.end());
	}
	if(node->chr == '*'){							// a star-node n = c1*
													// left == NULL; right != NULL
		node->firstpos.assign(node->right->firstpos.begin(), node->right->firstpos.end());		
	}
	// do nothing
}

// calculate the lastpos set of a given node
void calLastpos(SyntaxNode * node){
	
	// if(node->chr == 'E'){ do nothing }			// a leaf labeled empty string E
	
	
	if(node->pos > 0){								// a leaf with position i
		node->lastpos.push_back(node->pos);
	}
	if(node->chr == '|'){							// an or-node node = c1 | c2
		vector<int> temp(calUnion(node->left->lastpos, node->right->lastpos));
		node->lastpos.swap(temp);
	}
	if(node->chr == '-'){							// a cat-node n = c1c2
		if(nullable(node->right)){
			vector<int> temp(calUnion(node->left->lastpos, node->right->lastpos));
			node->lastpos.swap(temp);
		}
		else{
			node->lastpos.assign(node->right->lastpos.begin(), node->right->lastpos.end());
		}
	}
	if(node->chr == '+'){
		node->lastpos.assign(node->right->lastpos.begin(), node->right->lastpos.end());	
	}
	if(node->chr == '?'){
		node->lastpos.assign(node->right->lastpos.begin(), node->right->lastpos.end());	
	}
	if(node->chr == '*'){							// a star-node n = c1*
													// left == NULL; right != NULL
		node->lastpos.assign(node->right->lastpos.begin(), node->right->lastpos.end());		
	}
	//do nothing
} 

// calculate the followpos of the given pos
void calFollowpos(SyntaxNode * node, vector<set<int> > &followpos){
	if(node->chr == '-'){
		for(int i=0;i<node->left->lastpos.size();i++){
			for(int j=0;j<node->right->firstpos.size();j++){
				followpos[node->left->lastpos[i]].insert(node->right->firstpos[j]);
			}
		}
	}
	else if(node->chr == '*' || node->chr == '+'){
		for(int i=0;i<node->lastpos.size();i++){
			for(int j=0;j<node->firstpos.size();j++){
				followpos[node->lastpos[i]].insert(node->firstpos[j]);
			}
		}
	}
	// else { do nothing }
}

// analysis before constructing DFA
void initial_analysis(string reg, vector<char> & symbols, map<char, int> & lexer){
	set<char> help;													// store all possible symbols except for 'E' and '#'
	for(int i=0;i<reg.size();i++){
		if(reg[i] != 'E' && reg[i] != '#' && lexer[reg[i]]==5){	
			help.insert(reg[i]);
		}
	}
	symbols.assign(help.begin(), help.end());
	sort(symbols.begin(), symbols.end());							// sort for sequential indexing
	
}

// second and last analysis prepared for constructing DFA
void post_analysis(string reg, map<int, int> & all_pos, map<int, int> & reverse_pos, int & num_pos, map<char, int> & lexer){
	int pos = 1;									// start from 1
	for(int i=0;i<reg.size();i++){
		if(reg[i] != 'E' && lexer[reg[i]]==5){		// except for 'E'
			all_pos[i]= pos;						// original pos to labeled pos
			reverse_pos[pos] = i;					// labeled pos to original pos
			pos++;
		}
	}
	num_pos = pos-1;								// label is account
	
}

// judge whether two sorted symbol sets are equal
bool ifEqual(vector<char> & A, vector<char> & B){
	if(A.size()!=B.size()){
		return false;
	}
	for(int i=0;i<A.size();i++){
		if(A[i] != B[i]){
			return false;
		}
	}
	return true;
}

// judge whether two state sets are equal
bool ifEqual(vector<int> & A, vector<int> & B){
	if(A.size()!=B.size()){
		return false;
	}
	for(int i=0;i<A.size();i++){
		if(find(B.begin(), B.end(), A[i]) == B.end()){
			return false;
		}
	}
	return true;
}

// check if current states contain the particular state s and if exists return the marker of s
int findState(vector<vector<int> > & Dstates, vector<int> &s){
	int index = -1;												// -1 indicates not exist
	for(int i=0;i<Dstates.size();i++){
		if(ifEqual(Dstates[i], s)==true){
			index = i;
			break;
		}else{
			continue;
		}
	}
	return index;
}

// judge whether the state consist of positions contains the end_pos; if yes then state is of end_state otherwise not
bool ifEndstate(vector<int> & state, int end_pos){
	// original error: if(state.size()==0 && end_pos == 0){
	if(state.size() == 0){					//dead state is not of end state
		return false;
	}
	for(int i=0;i<state.size();i++){
		if(end_pos == state[i]){
			return true;
		}
	}
	return false;
}

// transform the regular expression reg to DFA, with the symbol set given at initial_analysis
void regexp2DFA(string reg, DFA & dfa, vector<char> & symbols, map<char, int> & lexer){
	
	//*-------------------- Step 1 ---------------------*//
	// construct syntax tree of regular expression //
	
	vector<char> op_stack;    					//  stack for operator
	vector<SyntaxNode*> syntax_tree_stack;   	//  stack for nodes of syntax tree

	// vector<char> symbols;					//  store all the mentioned symbols in reg
	map<int, int> all_pos;						//  store the <original pos, labeled pos> of non-E symbols in reg 
	map<int, int> reverse_pos; 					//  store the <labeled pos, original pos> of non-E symbols in reg 
	int num_pos = 0;							//  the important positions in regular expression

	post_analysis(reg, all_pos, reverse_pos, num_pos, lexer);	// second and last analysis of reg for preparations
	
	vector<set<int> > followpos(num_pos, set<int>());	// store followpos of each important pos; empty set for '#'

	char cur_chr = reg[0];								// store the character scanned just now

	for(int i=0;i<reg.length();i++){
		if(reg[i] == '(' ){							   					// deal with '('
			if(cur_chr != '(' && cur_chr != '|' && cur_chr != '-'){		// )( or [a-z]( or E( or *( or +( or ?( 
				op_stack.push_back('-');   								// concatenate 
			}
			op_stack.push_back('(');									// before pushing in stack, toppest priority
		}
		else if(reg[i] == ')'){											// deal with ')'
			char cur_op = op_stack.back();
			while(cur_op != '('){										// deal with the op in stack until meet the '('
				SyntaxNode* new_node = new SyntaxNode(cur_op);
										 
				new_node->right = syntax_tree_stack.back();
				syntax_tree_stack.pop_back();
				new_node->left = syntax_tree_stack.back();
				syntax_tree_stack.pop_back();
				
				// whenever construct a new non-leaf node
				calFirstpos(new_node);
				calLastpos(new_node);
				calFollowpos(new_node, followpos);

				syntax_tree_stack.push_back(new_node);
				new_node = NULL;
				op_stack.pop_back();
				cur_op = op_stack.back();
			} 
			op_stack.pop_back();										// pop '('
		}
		else if(reg[i] == '#'){	 										// end 
			while(op_stack.size()!=0){									// deal with the op remained in stack
				char old_op = op_stack.back();
				SyntaxNode* new_node = new SyntaxNode(old_op);
				op_stack.pop_back();									// pop 
																	// binary op
				new_node->right = syntax_tree_stack.back();
				syntax_tree_stack.pop_back();
				new_node->left = syntax_tree_stack.back();
				syntax_tree_stack.pop_back();
				
				calFirstpos(new_node);
				calLastpos(new_node);
				calFollowpos(new_node, followpos);
				
				syntax_tree_stack.push_back(new_node);
				new_node = NULL;
			}
			
			
			SyntaxNode* new_node = new SyntaxNode('-');			// concatenate with '#'
			new_node->left = syntax_tree_stack.back();
			new_node->right = new SyntaxNode('#', all_pos[i]);	
			
			calFirstpos(new_node->right);
			calLastpos(new_node->right);
			
			calFirstpos(new_node);
			calLastpos(new_node);
			calFollowpos(new_node, followpos);
				
			syntax_tree_stack.pop_back();
			if(syntax_tree_stack.size()!=0){
				
				exit(1);
			}
			syntax_tree_stack.push_back(new_node);
			new_node = NULL;
		}	
		else{		// deal with character in given symbol set, empty string, and the rest operators
			int lex = lexer[reg[i]];	
			if(lex == 5){   															// [a-z] | E  and impossible '#'
				if(i!=0){																// cur_chr should not be the first chr in reg
					if(cur_chr == ')' || lexer[cur_chr] == 4 || lexer[cur_chr] == 5){   // )a or .*a or .+a or .?a or aa/Ea
						op_stack.push_back('-');   										// concatenate 
					}
				}
				SyntaxNode* new_node = NULL;
				if(reg[i] != 'E'){
					new_node = new SyntaxNode(reg[i], all_pos[i]);			// leaf node
				}
				else{																	// no pos for E
					new_node = new SyntaxNode(reg[i]);						// leaf node
				}
				
				
				calFirstpos(new_node);
				calLastpos(new_node);
				syntax_tree_stack.push_back(new_node);
				new_node = NULL;
			}
			else if(lex == 4){		// unary op
				SyntaxNode* new_node = new SyntaxNode(reg[i]);
				new_node -> right = syntax_tree_stack.back();
				
				calFirstpos(new_node);
				calLastpos(new_node);
				calFollowpos(new_node, followpos);
				
				syntax_tree_stack.pop_back();
				syntax_tree_stack.push_back(new_node);
				new_node = NULL;
			}
			else{					// binary op
				if(op_stack.size()!=0){										// need to compare with previous op
					char old_op = op_stack.back();							// previous operator
					while(lexer[old_op]>=lex){								// priority comparision
						SyntaxNode* new_node = new SyntaxNode(old_op);		// non-leaf node of op
																		// binary operator
						new_node->right = syntax_tree_stack.back();		// left != NULL , right != NULL
						syntax_tree_stack.pop_back(); 
						new_node->left = syntax_tree_stack.back();	
						syntax_tree_stack.pop_back();
						
						calFirstpos(new_node);
						calLastpos(new_node);
						calFollowpos(new_node, followpos);
						
						syntax_tree_stack.push_back(new_node);
						new_node = NULL;
						
						op_stack.pop_back();								// pop op
						
						if(op_stack.size()!=0){								// continue merging
							old_op = op_stack.back();
						}
						else{
							break;
						}
					}
				}
				// no op in operator stack or lexer[old_op]<lex
				// push the current op	
				op_stack.push_back(reg[i]);	
			}
		}
		cur_chr = reg[i];	// store 
	}
	SyntaxNode * root = syntax_tree_stack.back();
	syntax_tree_stack.pop_back();
	
	//*-------------------- End of Step 1 ---------------------*//

	//*----------------------*-------*-------------------------*//
	
	//*---------------------- Step 2 --------------------------*//
	// 			   construct DFA with syntax tree     			//

	int num_states = 0;
	vector<vector<int> > Dstates;		// store states
	vector<vector<int> > Dtrans;		// store transition rules
	vector<int> end_states;				// store end states
	vector<int> buffer;					// store state consist of positions temporarily	

	// initial state: first(root)
	buffer.assign(root->firstpos.begin(), root->firstpos.end());
	Dstates.push_back(buffer);
	num_states++;
	if(ifEndstate(buffer, num_pos) == true){
		end_states.push_back(0);					// start state is also the end state
		
	}
	Dtrans.push_back(vector<int>(symbols.size(), -1));
	int cur_state = 0;
	
	
	while(cur_state < num_states){				// initial: 0 < 1
		buffer.assign(Dstates[cur_state].begin(), Dstates[cur_state].end());
		
		if(buffer.empty()){						// dead state
			for(int i = 0; i<symbols.size();i++){
				Dtrans[cur_state][i] = cur_state;
			}
		}
		else{
			for(int i = 0; i<symbols.size() ; i++){		// for every possible symbol [a-z]
				vector<int> pos_buffer;					// store all the pos in buffer(current state S) correspond to symbols[i]
				for(int j=0;j<buffer.size();j++){
					if(reg[reverse_pos[buffer[j]]] == symbols[i]){
						pos_buffer.push_back(buffer[j]);
					}
					
				}
				/* original error: if(pos_buffer.size() == 0){
					cout<<"empty!"<<endl;
					continue;							// no transition with symbols[i]
				}*/
				
				vector<int> the_union;					// "union" is of key word?
				for(int k=0 ; k<pos_buffer.size();k++){
					set<int> temp(followpos[pos_buffer[k]]);
					vector<int> tt(temp.begin(), temp.end());
					vector<int> t(calUnion(the_union, tt));
					the_union.swap(t);
				}
				

				int s_index = findState(Dstates, the_union);
				
				if( s_index == -1){						// new state
					Dstates.push_back(the_union);		// maybe empty
					num_states++;
					// cout<<"states : "<<num_states<<endl;
					Dtrans.push_back(vector<int>(symbols.size(), -1));
					s_index = num_states-1;
					if(ifEndstate(the_union, num_pos) == true){
						end_states.push_back(s_index);
					}
				}
				//  original problem: what if union is empty??
				Dtrans[cur_state][i] = s_index;			// [s][cur_symbol]
			}
			
		}
		
		cur_state++;								// next unmarked state;
		
	}
	
	// vector<vector<int> > & s, vector<vector<int> > & trans, vector<int> & es, vector<char> & sym, int num_states
	dfa = DFA(Dstates, Dtrans, end_states, symbols, num_states);
	
}

bool ifHave(vector<int> & v, int x){
	for(int i=0;i<v.size();i++){
		if(x==v[i]){
			return true;
		}
	}
	return false;
}

bool ifAccept(int s1, int s2, vector<int> &es1, vector<int> &es2){
	bool if1 = ifHave(es1, s1);
	bool if2 = ifHave(es2, s2);
	if(if1 && if2){
		return true;
	}
	else{
		return false;
	}
}

bool ifOverlap(DFA& A, DFA& B, int s1, int s2, int num_symbols, vector<int> &es1, vector<int> &es2, vector<vector<bool> > & visited){
	
	visited[s1][s2] = true;
	
	
	if(ifAccept(s1, s2, es1, es2)==true){
		
		return true;
	}
		
	
	for(int i=0;i<num_symbols;i++){
		int next_s1 = A.getNextStateName(s1, i);
		int next_s2 = B.getNextStateName(s2, i);
		
		
		if(!visited[next_s1][next_s2]){
			if(ifOverlap(A, B, next_s1, next_s2, num_symbols, es1, es2, visited)==true)
				return true;
			visited[next_s1][next_s2] = false;
		}
		
	}
	return false;
}


int main(){
	// store the priorities of symbols
	map<char, int> lexer;
	
	for(int i=0;i<26;i++){
		lexer['a'+i] = 5;
	}
	lexer['E'] = 5;
	lexer['#'] = 5;
	lexer['*'] = 4;
	lexer['+'] = 4;
	lexer['?'] = 4;
	lexer['-'] = 3;	// concatenate
	lexer['|'] = 2;
	lexer['('] = 1;
	int n;
	cin>>n;
	for(int i=0; i<n; i++){
		string r;
		string s;
		cin>>r;
		cin>>s;

		vector<char> symbolsA;
		vector<char> symbolsB;

		initial_analysis(r, symbolsA, lexer);
		initial_analysis(s, symbolsB, lexer);
		
		
		if(ifEqual(symbolsA, symbolsB) == false){
			cout<<"!"<<endl;
			continue;
		}
		
		DFA A;
		DFA B;
		regexp2DFA(r+"#", A, symbolsA, lexer);
		regexp2DFA(s+"#", B, symbolsB, lexer);
		
		// cout<<"construct dfa over"<<endl;

		int s1 = 0;
		int s2 = 0;
		vector<int> acceptA(A.getEndStates());
		vector<int> acceptB(B.getEndStates());

		vector<vector<bool> > visited(A.getNumStates(), vector<bool>(B.getNumStates(), false));

		vector<int> reverse_acceptA;
		vector<int> reverse_acceptB;
		A.reverseEnd(reverse_acceptA);
		B.reverseEnd(reverse_acceptB);

		int num_symbols = symbolsA.size();
		/* original error
		if(reverse_acceptA.size()!=0){
			s1 = reverse_acceptA.back();
			s2 = 0;
		}
		else{
			s1 = 0;
			s2 = 0;
		}
		*/
		
		bool one = ifOverlap(A, B, s1, s2, num_symbols, reverse_acceptA, acceptB, visited);
		/* original error
		if(reverse_acceptB.size()!=0){
			s1 = 0;
			s2 = reverse_acceptB.back();
		}
		else{
			s1 = 0;
			s2 = 0;
		}
		*/
		bool two = ifOverlap(A, B, s1, s2, num_symbols, acceptA, reverse_acceptB, visited);

		if(one && two){
			cout<<"!"<<endl;
		}
		else if(one){
			cout<<"<"<<endl;
		}
		else if(two){
			cout<<'>'<<endl;
		}
		else{
			cout<<"="<<endl;
		}
	}	
	return 0;
}
