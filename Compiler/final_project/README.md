### 问题描述

- 对于两个正则表达式 r 和 s，判断这两个正则表达式的关系。

  正则表达式r和s的关系有4种：

  1）r和s等价，即r描述的语言和s描述的语言相等；

  2）r描述的语言是s描述的语言的真子集；

  3）s描述的语言是r描述的语言的真子集；

  4）非上述情况。

- 正则表达式字符集：小写字母 ‘a’ - ‘z’；‘|’表示或者; ‘*’表示闭包，‘?’表示出现0或1次，‘+’表示至少出现一次，大写字母E表示epsilon（空串）

- 编写一个C++程序，实现上述功能

- 【输入】第一行是测试数组的组数T。

  接下来的T行，每行是两个正则表达式r和s，每个正则表达式只包含'a'-'z', '|', '*', '?', '+', 'E', '(', ')'。两个正则表达式之间用一个空格隔开。

- 【输出】输出有T行。对于每组数据，

  - 如果r和s等价，输出**=**；
  - 如果r是s的真子集，输出**<**；
  - 如果s是r的真子集，输出**>**；
  - 非上述情况，输出**!**。

------



### 实现思路

#### 1. 将正则表达式直接转为DFA

##### 1.1 构造拓展正则表达式的抽象语法树

- 拓展正则表达式 r = (r)#

- 构造抽象语法树

  - 语法树节点

  ```C++
  struct SyntaxNode{
  	char chr;
  	int pos;
  	vector<int> firstpos;
  	vector<int> lastpos;
  
  	SyntaxNode *left;
  	SyntaxNode *right;
  	SyntaxNode(char c=' ', int p=-1){
  		chr = c;
  		pos = p;	// if p == -1 node: chr is operator or E
  		left = NULL;
  		right = NULL;
  	}
  };
  ```

  - 语法树构造 (伪代码)

  ```pseudocode
  stack<SyntaxNode*> syntax_tree;
  stack<char> op_stack;	/* stack for operator */
  for x in r:
  	if x == '(':
  		if previous_x is ([a-z] or E or * or + or ?):
  			op_stack.push('-')	/* '-' for concatenate */
  		op_stack.push(x)
  	else if x == ')':
  		cur_op = stack.top();
  		while cur_op is not '(':
  			new_node = new SyntaxNode(chr = cur_op)
  			new_node->c2 = syntax_tree.top()
  			syntax_tree.pop()
  			new_node->c1 = syntax_tree.top()
  			syntax_tree.pop()
  			
  			calFirstpos(new_node);
              calLastpos(new_node);
              calFollowpos(new_node, followpos);
              
              syntax_tree.push(new_node)
              
              op_stack.pop()
              cur_op = stack.top()
          op_stack.pop()
      else if x == '#':
      	while op_stack is not empty:
      		old_op = op_stack.back()
      		new_node = new SyntaxNode(chr = old_op)
      		op_stack.pop()
      		
      		new_node->c2 = syntax_tree.top()
      		syntax_tree.pop()
      		new_node->c1 = syntax_tree.top()
      		syntax_tree.pop()
      		
      		calFirstpos(new_node);
              calLastpos(new_node);
              calFollowpos(new_node, followpos);
              
              syntax_tree.push(new_node)
              
  		new_node = 	new SyntaxNode('-')
  		new_node -> c1 = syntax_tree.top()
  		new_node -> c2 = new SyntaxNode('#', position('#'))
  		
  		calFirstpos(new_node->right);
          calLastpos(new_node->right);
  
          calFirstpos(new_node);
          calLastpos(new_node);
          calFollowpos(new_node, followpos);
          
          syntax_tree.pop()
          if syntax_tree is not empty:
          	raise ERROR
          
          syntax_tree.push(new_node)
      else:
      	if x is ( [a-z] or E ):
      		if previous_x is ( ')' or * or ? or + or [a-z] or E ):
      			op_stack.push('-')		/* concatenate */
      		if x != 'E':
      			new_node = new SyntaxNode(x, position(x))
      		else:
      			new_node = new SyntaxNode(E)	/* no position for E */
      		calFirstpos(new_node);
  			calLastpos(new_node);
  			syntax_tree.push(new_node)
  		else if x is ( * or ? or + ):	/* unary op */
  			new_node = new SyntaxNode(old_op)
  			new_node -> c2 = syntax_tree.top()
  			
  			calFirstpos(new_node);
              calLastpos(new_node);
              calFollowpos(new_node, followpos);
              
              syntax_tree.pop()
              syntax_tree.push(new_node)
          else: /* binary op */
          	if op_stack is not empty:
          		old_op = op_stack.back()
                  while priority[old_op] >= priority[x]:
                      new_node = new SyntaxNode(old_op)
                      new_node -> c2 = syntax_tree.top()
                      syntax_tree.pop()
                      new_node -> c1 = syntax_tree.top()
                      syntax_tree.pop()
  
                      calFirstpos(new_node);
                      calLastpos(new_node);
                      calFollowpos(new_node, followpos);
  
                      syntax_tree.push(new_node)
                      op_stack.pop()
  
                      if op_stack is not empty:
                          old_op = op_stack.top()
                      else:
                          break
  				end while
  			end
  		end
               /* no op in operator stack or priority[old_op]< priority[x]*/
  		op_stack.push(x)
  		end
  	end
      previous_x = x
  end
  		
  root = syntax_tree.top() /* the final node remained in node stack is the root of syntax tree */
  syntax_tree.pop()
  ```

  

##### 1.2 计算nullable(n)，firstpos(n)，lastpos(n)，followpos(p) (实现中与语法树构造同时进行)

- nullable(n)，firstpos(n)，lastpos(n)

  | NODE n                 | nullable                      | firstpos                                                     | lastpos                                                      |
  | ---------------------- | ----------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
  | A leaf labeled E       | true                          | {}                                                           | {}                                                           |
  | A leaf with position i | false                         | {i}                                                          | {i}                                                          |
  | An or-node n = c1\|c2  | nullable(c1) or nullable(c1)  | firstpos(c1) ∪ firstpos(c2)                                  | lastpos(c1) ∪ lastpos(c2)                                    |
  | A cat-node n = c1c2    | nullable(c1) and nullable(c1) | if( nullable(c1) )<br/>{ firstpos(c1) ∪ firstpos(c2); }<br/>else{ firstpos(c1); } | if( nullable(c2) )<br/>{ lastpos(c1) ∪ lastpos(c2); }<br />else{ lastpos(c2); } |
  | A star-node n = c2*    | true                          | firstpos(c2)                                                 | lastpos(c2)                                                  |
  | n = c2+                | nullable(c2)                  | firstpos(c2)                                                 | lastpos(c2)                                                  |
  | n = c2？               | true                          | firstpos(c2)                                                 | lastpos(c2)                                                  |

  

- followpos(p)

  |                     |                                                              |
  | ------------------- | ------------------------------------------------------------ |
  | A cat-node n = c1c2 | for( p1 in c1->lastpos){<br/>	for(p2 in c2->firstpos){<br/>		followpos[p1].insert(p2);<br/>	}<br/>} |
  | A star-node n = c1* | for(p1 in n->lastpos){<br/>	for(p2 in n->firstpos){<br/>		followpos[p1].insert(p2);<br/>	}<br/>} |
  | n = c1+             | for(p1 in n->lastpos){<br/>	for(p2 in n->firstpos){<br/>		followpos[p1].insert(p2);<br/>	}<br/>} |
  | n = c1?             | nothing can be inferred                                      |

  

##### 1.3 正则表达式直接转DFA算法

- DFA 对象

```C++
class DFA{	
	private:
		vector<vector<int> > Dstates;		// store states (index of dim-0 as marker) 
		vector<vector<int> > Dtrans;		// store transition rules ; size = num_states * num_symbols
		vector<int> end_states;				// store the markers of end states
		vector<char> symbols;				// symbol set
		int num_states;						// number of states in dfa 
	
	public:
		DFA(vector<vector<int> > & s, vector<vector<int> > & trans, vector<int> & es, vector<char> & sym, int num_states);
		DFA();
		DFA(const DFA &obj);
		
		int getNumStates();
		int getNumSymbols();
		int getNextStateName(int s, int chr);
		vector<int> getState(int i);
		vector<char> getSymbols();
		vector<int> getEndStates();
		// get all the states that of non-accepted states
		void reverseEnd(vector<int> & reverse);
		
};
```

- 正则表达式转DFA

  ```pseudocode
  INPUT: a regular expression r
  OUTPUT: a DFA D that recognizes L(r)
  
  Dstates = { firstpos(root) }
  num_states = 1
  dead_state: a state without any pos and is not accept state
  end_states: the set of accept state
  /*-----------------------------------------------------------------------------------------------------*/
  while there is an unmarked state S in Dstates:
  	mark S
  	if S is dead_state:	
  		for each symbol a in symbol_set:
  			Dtrans[S, a] = S
  	end
  	else:
          for each symbol a in symbol_set:
              U = calUnion( followpos(p) for p in state(S) that correspond to a)
              if U is not in Dstates:
                  add U as an unmarked state to Dstates
                  num_states++
              end
  			if U is of end_states:
  				add U to end_states
  			Dtrans[S, a] = U
  			end
  		end
  	end
  end
  
  dfa = DFA(Dstates, Dtrans, end_states, symbols, num_states)
  ```

  

##### 1.4 特殊处理

- 处理对于某个可能的输入符号symbol，没有状态转移的情况——加入一个“死状态”，所有没有状态转移的情况对应转移状态为“死状态”，“死状态”对应的所有symbol的状态迁移指向自身



#### 2. 判断两个DFA的关系

- 判断两个DFA是否相交的方法

  ```pseudocode
  INPUT = {
  	A: DFA(r),
  	B: DFA(s),
  	s1: start state marker of A,
  	s2: start state marker of B,
  	es1: end states set of A,
  	es2: end states set of B,
  	visited: visit record
  }
  OUTPUT = {
  	true if intersection(A, B) is not empty,
  	false otherwise
  }
  bool ifOverlap(INPUT):
  	visited[s1][s2] = true
  	if ifAccept(s1, s2, es1, es2):
  		return true
  	end
  	for index in range(num_symbols):
  		next_s1 = A[s1][index]
  		next_s2 = B[s2][index]
  		if not visited[next_s1][next_s2]:
  			NEW_INPUT = INPUT
  			NEW_INPUT.s1 = next_s1
  			NEW_INPUT.s2 = next_s2
  			if ifOverlap(NEX_INPUT) == true:
  				return true
  			end
  			visited[next_s1][next_s2] = false
  		end
  	end
  	return false
  ```

- 求 DFA A 的补集 A’：原DFA中的所有接收状态变为非接收状态，原接收状态转为非接收状态

##### 2.1 对于dfa A，dfa B的补集B’，判断 A ∩ B’ 是否为空集

##### 2.2 对于dfa A 的补集A‘，dfa B，判断 A' ∩ B 是否为空集

##### 2.3 根据2.1和2.2的结果判断两个DFA的关系

- **若 2.1 && 2.2  则r与s既不等价也不满足一方为另一方的真子集**
- **若 2.1 && !2.2 则r是s的真子集**
- **若 !2.1 && 2.2 则s是r的真子集**
- **若 !2.1 && !2.2 则r与s等价**

