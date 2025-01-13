#include "rl3.h"




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

void updateActionOfS(int i,int j,double reward[][gridC],int action[][gridC],Matrix *V)
{
    int s=state[i][j],s2,maxAxtionDir;
    double maxActionVal=-99999;//最大动作值，最大动作值的方向
    for(int k=0;k<actionNum;k++)
    {
        int nextr=i+dr[k],nextc=j+dc[k];//检查采取第k种动作的动作值
        double curActionVal;
        if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC)//撞墙返回
        {
            // s2=s;
            curActionVal=rBoundry+gamma*V->a[s][0];
        }
        else 
        {
            s2=state[nextr][nextc];
            curActionVal=reward[nextr][nextc]+gamma*V->a[s2][0];
        }
        if(curActionVal>=maxActionVal)
        {
            maxActionVal=curActionVal;
            maxAxtionDir=k;
        }
    }
    action[i][j]=maxAxtionDir;
    return;
}

void actionImprovement(double reward[][gridC],int action[][gridC],Matrix* V)
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            updateActionOfS(i,j,reward,action,V);
        }
    }
    showAction(action);
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
    // printf("wrong \n");
    return actionNum-1;//实际程序不应该运行到这里
}


void genRoad(Matrix* policy,std::vector<saPair> *road,int stepNum)
{
    int initS=std::rand()%numStates,initAction=std::rand()%actionNum;//随机初始化(s,a)对
    // int initS=0,initAction=std::rand()%actionNum;;
    // saPair start=genInitSaPair();
    // int initS=start.s,initAction=start.a;
    // int initS=state[initR][initC];
    // int initS=0,initAction=0;
    road->push_back({initS,initAction});
    for(int i=1;i<stepNum;i++)
    {
        int s=(*road)[i-1].s,a=(*road)[i-1].a;
        int r=s/gridC,c=s%gridC;
        int nextr=r+dr[a],nextc=c+dc[a];
        int nextAction;
        if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC)//撞墙
        {
            nextAction=getRandomAction(policy,s);
            road->push_back({s,nextAction});
        }
        else 
        {
            int nextS=state[nextr][nextc];
            nextAction=getRandomAction(policy,nextS);
            road->push_back({nextS,nextAction});
        }
    }
}

void policyEvaluate(double reward[][gridC],Matrix* q,std::vector<saPair> *road)
{
    double returns[numStates][actionNum];
    int count[numStates][actionNum];//为了计算平均值需要记录访问的次数
    memset(returns,0,sizeof(returns));
    // for(int i=0;i<numStates;i++)
    //     for(int j=0;j<actionNum;j++)
    //         returns[i][j]=-9999;//初始化为-
    memset(count,0,sizeof(count));
    double g=0.0;
    for(int i=road->size()-1;i>=0;i--)
    {
        int s=(*road)[i].s,a=(*road)[i].a;
        int r=s/gridC,c=s%gridC;
        int nextr=r+dr[a],nextc=c+dc[a];
        double re;
        if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC)//是否撞墙
            re=rBoundry;
        else re=reward[nextr][nextc];
        g=gamma*g+re;
        returns[s][a]+=g;
        count[s][a]++;
    }
    // showCount(count);
    for(int i=0;i<numStates;i++)
        for(int j=0;j<actionNum;j++){
            if(count[i][j]){
                q->a[i][j]=alpha*(returns[i][j]/count[i][j])+(1-alpha)*q->a[i][j];
            }
        }
}

void policyImprovement(Matrix *q,Matrix *policy,double epsilon)
{
    double bestPolicyProb=1.0-epsilon*((actionNum-1.0)/actionNum);//这里要写成1.0!!!!!!!! instead of 1
    double otherPolicyProb=epsilon*(1.0/actionNum);
    for(int i=0;i<numStates;i++)
    {
        double maxqVal=-9999;
        int maxqIndex=std::rand()%actionNum;;//如果某种状态未被采样，随机走一个方向
        for(int j=0;j<actionNum;j++)
        {
            // if(count[i][j]&&q->a[i][j]>maxqVal)
            if(q->a[i][j]>maxqVal)
            {
                maxqVal=q->a[i][j];
                maxqIndex=j;
            }
        }
        for(int j=0;j<actionNum;j++)
        {
            // if(j==maxqIndex)policy->a[]
            policy->a[i][j]=j==maxqIndex?bestPolicyProb:otherPolicyProb;
        }

    }
}

void epsilonGreed(double reward[][gridC],double epsilon,int episodeNum,int stepNum)
{
    Matrix policy(numStates,actionNum);//policy[i][j]表示状态i采取动作j的概率
    Matrix q(numStates,actionNum);//q[i][j]表示状态i采取动作j的动作值
    policy.uniformInit();
    // for(int i=0;i<numStates;i++)
    // {
    //     policy.a[i][0]=1.0;
    //     for(int j=1;j<actionNum;j++)
    //     {
    //         policy.a[i][j]=0.0;
    //     }
    // }
    // policy.show();
    // stepNum=10000000;
    double targetEpsilon=epsilon,deltaEpsilon=1.0-epsilon;
    epsilon=targetEpsilon;//epsilon从1.0开始慢慢降到targetEpsilon，程序初期鼓励探索，后期加快收敛
    std::vector<saPair> road;//已经为vector声明容量后，继续push_back会在第stepnum个元素后加入新的元素
    Matrix V(numStates,1);
    Matrix prevV(numStates,1);
    int i;
    for(i=0;i<episodeNum;i++)
    {
        road.clear();
        if(i<2)
            genRoad(&policy,&road,10000000);
        // else genRoad(&policy,&road,stepNum);
        // std::cout<<stepNum<<"!!!!!!"<<std::endl;
        else genRoad(&policy,&road,stepNum);
        stepNum=stepNum/2+100000;

        policyEvaluate(reward,&q,&road);
        policyImprovement(&q,&policy,epsilon);

        if(i<8){
            // deltaEpsilon/=2;
            epsilon=targetEpsilon+deltaEpsilon/i;
        }
        else epsilon=targetEpsilon;
        // std::cout<<epsilon<<std::endl;
        // getVByPolicy(&q,&policy,&V);
        // if(getDiff(&V,&prevV)<0.0001&&i!=0)
        //     break;
        // prevV.copy(&V);
        if(i%1000==0){
            std::cout<<i<<"!!!"<<std::endl;
        // // showRoad(&road);
        // // showAllPolicy(&q);
        // // showAllPolicy(&policy);
        // showVByPolicy(&q,&policy);
        // // showMaxPolicyInGrid(&policy);
        }

        // std::cout<<"!!!!!!!!!!!3"<<std::endl;
    }
        showRoad(&road);
        showAllPolicy(&q);
        // showAllPolicy(&policy);
        showVByPolicy(&q,&policy);
        getV(&policy,reward);
        showMaxPolicyInGrid(&policy);
        printf("iter end i:%d !\n",i);
}

int main()
{

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    double reward1[gridR][gridC]={
        {rOtherstep,rOtherstep,rOtherstep,rOtherstep,rOtherstep},
        {rOtherstep,rForbidden,rForbidden,rOtherstep,rOtherstep},
        {rOtherstep,rOtherstep,rForbidden,rOtherstep,rOtherstep},
        {rOtherstep,rForbidden,rTarget,rForbidden,rOtherstep},   
        {rOtherstep,rForbidden,rOtherstep,rOtherstep,rOtherstep},
    };



    // epsilonGreed(reward1,0.0,1000,10000000);
    // epsilonGreed(reward1,0.2,2000,1000000);
    epsilonGreed(reward1,0.1,2000,1000000);
    // epsilonGreed(reward1,0.5,1000,10000000);
}