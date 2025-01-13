#include "head.h"




void show(Matrix *A)
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            printf("%10.4f ",A->a[state[i][j]][0]);
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

void show2(Matrix *A)
{
    for(int i=0;i<numStates;i++)
    {
        for(int j=0;j<numStates;j++)
        {
            std::cout<<A->a[i][j]<<' ';
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

void showAction(int a[][gridC])
{
    // char dir[actionNum]="↑→↓←.\0";
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            if(a[i][j]==0)std::cout<<"up    ";
            else if(a[i][j]==1)std::cout<<"right ";
            else if(a[i][j]==2)std::cout<<"down  ";
            else if(a[i][j]==3)std::cout<<"left  ";
            else if(a[i][j]==4)std::cout<<"stay  ";
            std::cout<<" ";
        }
        std::cout<<std::endl;
    }
}


double getDiff(Matrix *V,Matrix *nextV)
{
    double maxDiff=0;
    for(int i=0;i<numStates;i++)
    {
        double diff=V->a[i][0]-nextV->a[i][0];
        double absDiff=diff>0?diff:-diff;
        maxDiff=maxDiff>absDiff?maxDiff:absDiff;
    }
    // printf("%f\n",maxDiff);
    return maxDiff;
}


void getV(Matrix *policy,double reward[][gridC])
{
    Matrix P(numStates,numStates);
    Matrix R(numStates,1);
    P.zero();
    R.zero();
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            int s1=state[i][j];
            for(int k=0;k<actionNum;k++)
            {
                int nextr=i+dr[k],nextc=j+dc[k];

                if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC)
                {
                    P.a[s1][s1]+=policy->a[s1][k];
                    R.a[s1][0]+=policy->a[s1][k]*rBoundry;
                }
                else
                {
                    int s2=state[nextr][nextc];
                    P.a[s1][s2]+=policy->a[s1][k];
                    R.a[s1][0]+=reward[nextr][nextc]*policy->a[s1][k];
                }
            }
        }
    }

    Matrix V(numStates,1);
    Matrix inverse(numStates,numStates);
    Matrix I(numStates,numStates);//对角矩阵
    I.eye();
    Matrix temp(numStates,numStates);

    P.kMultiply(gamma);
    matrixSub(&I,&P,&temp);
    
    // show2(&temp);
    matrixInverse(&temp,&inverse);

    matrixMul(&inverse,&R,&V);

    show(&V);
}

int getRandomAction(Matrix* policy,int s)
{
    double randomValue = static_cast<double>(rand()) / RAND_MAX;
    // std::cout<<randomValue<<std::endl;
    double probabilitySum=0.0;
    for(int i=0;i<actionNum;i++)
    {
        probabilitySum+=policy->a[s][i];
        if(probabilitySum>=randomValue)return i;
    }
    return actionNum-1;
}



void offPolicyQLearning(double reward[][gridC],int episodeNum)
{
    Matrix policy(numStates,actionNum);//policy[i][j]表示状态i采取动作j的概率，行为策略
    Matrix targetPolicy(numStates,actionNum);//目标策略
    Matrix q(numStates,actionNum);//q[i][j]表示状态i采取动作j的动作值
    policy.uniformInit();
    q.uniformInit();//初始猜测所有q(s,a)都为0
    int i;
    int stepNum=100000;
    // showAllPolicy(&policy);
    for(i=0;i<episodeNum;i++)
    {
        int s=std::rand()%numStates;//随机初始化s0
        int a=getRandomAction(&policy,s);//{s0, a0, r1, s1 a1, r2,
        for(int j=0;j<stepNum;j++)
        {
            int r=s/gridC,c=s%gridC;
            int nextr=r+dr[a],nextc=c+dc[a];
            int nextAction,nextS;
            double nextReward=0;
            if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC)//撞墙
            {
                nextAction=getRandomAction(&policy,s);
                nextS=s;
                nextReward=rBoundry;
            }
            else 
            {
                nextS=state[nextr][nextc];
                nextAction=getRandomAction(&policy,nextS);
                nextReward=reward[nextr][nextc];
            }
            double maxQ=q.a[nextS][0];
            int maxQAction=0;
            double targetMaxQ=q.a[s][0];
            int targetmaxQAction=0;
            for(int k=1;k<actionNum;k++)
            {
                if(q.a[nextS][k]>=maxQ)
                {
                    maxQ=q.a[nextS][k];
                    maxQAction=k;
                }
                if(q.a[s][k]>=targetMaxQ)
                {
                    targetMaxQ=q.a[s][k];
                    targetmaxQAction=k;
                }
            }
            q.a[s][a]=q.a[s][a]-alpha*(q.a[s][a]-(nextReward+gamma*maxQ));


            memset(targetPolicy.a[s],0,sizeof(double)*actionNum);
            targetPolicy.a[s][targetmaxQAction]=1.0;
            if(j%10000==0)
            {
                // std::cout<<"s:"<<s<<" a:"<<a<<" nextS:"<<nextS<<" nextA:"<<nextAction<<std::endl;
                // showAllPolicy(&targetPolicy);
                // std::cout<<std::endl;
                // showAllPolicy(&q);
                // std::cout<<std::endl;
                // showMaxPolicyInGrid(&targetPolicy);
                // getV(&targetPolicy,reward);
            }
            s=nextS;
            a=nextAction;
        }
        showMaxPolicyInGrid(&targetPolicy);
        getV(&targetPolicy,reward);
    }
}

int main()
{

    // std::srand(static_cast<unsigned int>(std::time(nullptr)));

    double reward1[gridR][gridC]={
        {rOtherstep,rOtherstep,rOtherstep,rOtherstep,rOtherstep},
        {rOtherstep,rForbidden,rForbidden,rOtherstep,rOtherstep},
        {rOtherstep,rOtherstep,rForbidden,rOtherstep,rOtherstep},
        {rOtherstep,rForbidden,rTarget,rForbidden,rOtherstep},   
        {rOtherstep,rForbidden,rOtherstep,rOtherstep,rOtherstep},
    };

    offPolicyQLearning(reward1,1);
}