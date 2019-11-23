
/*

if you want to see what the prog does in more details

please uncomment lines 124,126, and 144

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h> 
#include <unistd.h>
#include <string.h>
#include <stdbool.h> 
#define Max 100000
#define NumberOfPaths 4


struct bat
{
    int num;
    int dir;
};

pthread_mutex_t CrossMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t QMutex[NumberOfPaths];

pthread_cond_t  CrossCond [NumberOfPaths];

pthread_cond_t QCond [NumberOfPaths];

int WaitQ[NumberOfPaths];

bool WaitCross[NumberOfPaths];

bool WaitCrossing[NumberOfPaths];

bool DoneChecking=false;

int getdir(char a){
    switch(a){
        case 'n':
            return 0;
        case 's':
            return 1;
        case 'e':
            return 2;
        case 'w':
            return 3;
    }
    return 0;
}

char* getst(int a){
    switch(a){
        case 0:
            return "North";
        case 1:
            return "South";
        case 2:
            return "East";
        case 3:
            return "west";
    }
    return "error";
}

void initializeBats(struct bat bats[], char* str){
    int n=strlen(str);
    for(int i=0;i<n;i++){
        bats[i].num=i;
        bats[i].dir=getdir(str[i]);
    }
}

void initializeCondAndQs(){
    for (int i=0;i<NumberOfPaths;i++){
        CrossCond[i]= (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        QCond[i]= (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        QMutex[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        WaitQ[i]=0;
        WaitCross[i]=false;
        WaitCrossing[i]=false;
    }
}

int rightOf(int t){
    switch(t){
        case 0:
            return 3; // n --> w
        case 1:
            return 2; // s --> e
        case 2:
            return 0; // e --> n
        case 3:
            return 1; // w --> s
    }
    return 0;
}

int revRightOf(int t){
    switch(t){
        case 3:
            return 0; // n <-- w
        case 2:
            return 1; // s <-- e
        case 0:
            return 2; // e <-- n
        case 1:
            return 3; // w <-- s
    }
    return 0;
}



void arrive(struct bat b){
    pthread_mutex_lock(&QMutex[b.dir]);
    if(WaitQ[b.dir]>0||WaitCross[b.dir]||WaitCrossing[b.dir]){
        WaitQ[b.dir]++;
 //       printf("%s waiten\n",getst(b.dir));
        pthread_cond_wait(&QCond[b.dir],&QMutex[b.dir]);
 //       printf("%s is signeled from queue\n",getst(b.dir) );
        WaitQ[b.dir]--;
    }
    
    
    printf("BAT %d from %s arrives at crossing\n", b.num, getst(b.dir));
    WaitCross[b.dir]=true;
    WaitCrossing[b.dir]=true;
    pthread_mutex_unlock(&QMutex[b.dir]);
    // code to check traffic in line, use counters, condition variables
    
}

void cross(struct bat b){
    // code to check traffic from the right, use counters, condition variables etc
    pthread_mutex_lock(&CrossMutex);
    if(WaitCross[rightOf(b.dir)]||WaitCrossing[rightOf(b.dir)]){
        pthread_cond_wait(&CrossCond[b.dir],&CrossMutex);
//        printf("%s is signeled from cross\n",getst(b.dir) );
    }
    printf("BAT %d from %s crossing\n", b.num, getst(b.dir));
    WaitCross[b.dir]=false;
    sleep(1); 
    // it takes one second for a BAT to cross
    
}

void leave(struct bat b){
    printf("BAT %d from %s leaving crossing\n", b.num, getst(b.dir));
    WaitCrossing[b.dir]=false;
    pthread_cond_signal(&CrossCond[revRightOf(b.dir)]);
    pthread_cond_signal(&QCond[b.dir]);
    pthread_mutex_unlock(&CrossMutex);
    // code to check traffic, use counters, condition variables etc.
}

void* batman(void* arg){

    

    struct bat *b = (struct bat*) arg;

    arrive(*b);
    
    cross(*b);

    leave(*b);
    
    return 0;
}

void* check(void* arg){
    bool done=false;
    while(1){
        sleep(1);

        if(WaitCross[0]&&WaitCross[1]&&WaitCross[2]&&WaitCross[3]&&!done){
            printf("DEADLOCK: BAT jam detected, signalling any waiting thread\n");
               printf("%dddscsdccsdcdscs\n", pthread_cond_signal(&CrossCond[0])); 
            pthread_cond_signal(&CrossCond[0]);

            done=true;
        }
        if(WaitCross[0]==false||WaitCross[1]==false||WaitCross[2]==false||WaitCross[3]==false){
            done=false;
        }
        if(DoneChecking)
            return 0;
    }
    return 0;
}

void checking(){
    bool done=false;
    if(WaitCross[0]&&WaitCross[1]&&WaitCross[2]&&WaitCross[3]&&!done){
    printf("DEADLOCK: BAT jam detected, signalling North to go\n");
    pthread_cond_signal(&CrossCond[0]);
    done=true;
    }
    if(WaitCross[0]==false||WaitCross[1]==false||WaitCross[2]==false||WaitCross[3]==false){
        done=false;
    }
    if(DoneChecking)
        return;

}

int main(int argc, char **argv)
{
	
	//int num_args = argc - 1;
    if(argc!=2)
        return 1;

    char* str=argv[1];
    int n = strlen(str);
	
    struct bat bats[n];

    initializeBats(bats,str);
    initializeCondAndQs();
    
	// Launch threads
    pthread_t t;
    pthread_create(&t, NULL, check, NULL);

    pthread_t tid[n];
    for(int i=0;i<n;i++){
        pthread_create(&tid[i], NULL, batman, &bats[i]);
    }
    
    for(int i=0;i<n;i++){
        pthread_join(tid[i],NULL);
    }
    DoneChecking=true;
    pthread_join(t,NULL);
	
}
