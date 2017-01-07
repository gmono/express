#include <iostream>
#include "expresscore.hh"
int main(int argc, char *argv[])
{
    start:
    string text;
    cout<<"please enter a expression:";
    cin>>text;
    int ptr=0;//从开头开始
    ExpressCore core;
    //这里添加+ - 运算符
    //我只能说……前面的是B 后面的是A
    core.registSymbol("+",1,[](int a,int b)->int{return a+b;});
    core.registSymbol("-",1,[](int a,int b)->int{return a-b;});
    core.registSymbol("*",2,[](int a,int b)->int{return a*b;});
    core.registSymbol("/",2,[](int a,int b)->int{return a/b;});
//    core.registFunction("test",[](vector<int> pars){
//        int sum=0;
//        for(int i=0;i<pars.size();++i) sum+=pars[i];
//        return sum;
//    });
    //以上注册两个运算符
    int res=core.readExpression(ptr,text,"=");//以等于结束 或者以ptr大于等于字符串长度为结束
    cout<<res<<endl;
    //逆波兰式处理
    goto start;
    return 0;
}
