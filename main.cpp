#include <iostream>
#include "expresscore.hh"
typedef double VT;
int main(int argc, char *argv[])
{
    start:
    string text;
    cout<<"please enter a expression:";
    cin>>text;
    int ptr=0;//从开头开始
    ExpressCore<VT> core;
    //这里添加+ - 运算符
    //我只能说……前面的是B 后面的是A
    core.registSymbol("+",1,[](VT a,VT b)->VT{return a+b;});
    core.registSymbol("-",1,[](VT a,VT b)->VT{return a-b;});
    core.registSymbol("*",2,[](VT a,VT b)->VT{return a*b;});
    core.registSymbol("/",2,[](VT a,VT b)->VT{return a/b;});
    core.registSymbol("^^",3,[](VT a,VT b)->VT{
        //a^^b =a*(a-1)*...总共循环b次
        VT sum=a--;
        for(int i=1;i<b;++i) sum*=a--;
        return sum;
    });
    core.registFunction("test",[](vector<VT> pars){
        VT sum=0;
        for(int i=0;i<pars.size();++i) sum+=pars[i];
        return sum;
    });
    //以上注册两个运算符
    VT res=core.readExpression(ptr,text,"=");//以等于结束 或者以ptr大于等于字符串长度为结束
    cout<<res<<endl;
    //逆波兰式处理
    goto start;
    return 0;
}
