#通用解释器内核
此解释器采用C++编写
目前实现了表达式计算和函数注册与调用
关键字注册符号识别等
目前只支持双目运算符
#发展
实现了for into循环
实现了RunBlock函数
#未来展望
预计将修改runExpression函数
统一使用getExp函数获取单词
以方便未来维护和控制
同时getExp可以保证自动跳过前后空格
减少BUG发生率
同时减少代码量
##算法问题
RunExpression函数中用的是原始逆波兰算法
各方面性能有待优化
预计未来更深入理解逆波兰后进行修改
预计将大幅降低代码量