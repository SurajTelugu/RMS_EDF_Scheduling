#include <iostream>
#include <queue>
#include <limits.h>

using namespace std;

//Struct storing information of process (Process Control Block)
struct Process
{
  unsigned pid;    // process id   
  unsigned t;      // Burst time
  unsigned p;      // period
  unsigned k;      // Number of times process has to be executed
  int remk;        // Number of remaining times process has to be executed
  unsigned next_dl;
  unsigned arrt;   // Arrival time
  unsigned remt;   // Remaining burst time
  double wait;   //  Average Waiting time for each process
  bool chk_prempt; // Checks whether process is preempted or not
};
typedef struct Process INPUT;

struct Process_Priority
{
  unsigned pid;
  unsigned p;
};
typedef struct Process_Priority prcs_pr;

//Comparing function for priority queue (min heap based on periods)
struct Compare_Periods 
{
    bool operator()(prcs_pr const& n1,prcs_pr const& n2)
    {
      if(n1.p!=n2.p)
      {return !(n1.p< n2.p);}
       else
       {return !(n1.pid < n2.pid);}
    }
};

int Waitingtime_formula(INPUT* run_prcs, unsigned time)
{return (time - run_prcs->arrt - run_prcs->t + run_prcs->remt);}

// Earliest Deadline Sheduling Algorithm 
void EDF_Sheduler(unsigned N , INPUT* prcs)
{ 
  
  priority_queue<prcs_pr,vector<prcs_pr>,Compare_Periods> run_pq, next_pq;

  // Intialising file pointer and opening file for RMS-Log.txt file
    FILE* edf_log;
    edf_log = fopen("EDF-Log.txt","w");

  // Pushing processes into priority queue
  for(int i=0;i<N;i++)
  {
   if(prcs[i].k != 0) 
   {next_pq.push({prcs[i].pid,prcs[i].arrt});}
  }

  fprintf(edf_log,"Process P%u: processing time=%u; deadline:%u; period:%u joined the system at time %u\n"
          ,prcs[0].pid,prcs[0].t,prcs[0].p,prcs[0].p,prcs[0].arrt);
  
  // Outputing the Processes joining in system
  for(int i=1;i<N;i++)
  {
    // Outputing the Processes joining in system
   fprintf(edf_log,"Process P%u: processing time=%u; deadline:%u; period:%u joined the system at time %u\n"
          ,prcs[i].pid,prcs[i].t,prcs[i].p,prcs[i].p,prcs[i].arrt);
  }
  
  // Intial time = 0
  unsigned time = 0; 
  unsigned p;
  unsigned deadline_missed = 0;
   
   while(!run_pq.empty() || !next_pq.empty())
   {
     if(!next_pq.empty() && time == next_pq.top().p)
     {
       p = next_pq.top().pid;
       INPUT* run_prcs = &prcs[p-1];
       run_prcs->next_dl = time + run_prcs->p;

       if(run_prcs->remk > 0)
       {
         if(run_prcs->remt != 0)
         {
           run_prcs->wait = run_prcs->wait + run_prcs->p;
           run_prcs->arrt = time;
           deadline_missed++;
           run_pq.pop();
           run_pq.push({p,run_prcs->next_dl});
           fprintf(edf_log,"Process%u misses its deadline at time %u.\n",p,time);
         }

         else
         {
           run_prcs->arrt = time;
           run_pq.push({p,run_prcs->next_dl});
         }
       }

       run_prcs->remk--;
       run_prcs->remt = run_prcs->t;
       run_prcs->next_dl = time + run_prcs->p;
       next_pq.pop();

       if(run_prcs->remk > 0)
       {next_pq.push({p,run_prcs->next_dl});}
       continue;  
     }

     
     if (run_pq.empty()) 
     { 
      if (next_pq.empty()) break;
      else 
      { 
        time = next_pq.top().p;
        if (prcs[next_pq.top().pid-1].remk != 0)
        {fprintf(edf_log,"CPU is idle till time %u.\n",time-1);}
        continue;
      }
     } 
    else 
    { // ready_q not empty
      p = run_pq.top().pid;
      INPUT* run_prcs = &prcs[p-1];

      if (run_prcs->remt + time > run_prcs->next_dl) 
      { 
        run_prcs->remt = 0;
        deadline_missed++;
        run_prcs->wait = run_prcs->wait + run_prcs->p;
        prcs[run_pq.top().pid-1].next_dl = INT_MAX;
        run_pq.pop();
        continue;
      }

      if (run_prcs->t == run_prcs->remt) 
      {
        run_prcs->chk_prempt = false;
        if(time==0) fprintf(edf_log,"Process P%u starts execution at time %u.\n.",p,time);
        else fprintf(edf_log,"Process P%u starts execution at time %u.\n",p,time + 1);
      }
      else if (run_prcs->chk_prempt) 
      {
        run_prcs->chk_prempt = false;
        fprintf(edf_log,"Process P%u resumes execution at time %u.\n",p,time + 1);
      }

      if (next_pq.empty()) 
      {
        if (run_prcs->remt + time <= run_prcs->next_dl) 
        {
          run_prcs->wait = run_prcs->wait + Waitingtime_formula(run_prcs,time);
          time = time + run_prcs->remt;
          run_prcs->next_dl = run_prcs->next_dl + run_prcs->p;
          fprintf(edf_log,"Process P%u finishes execution at time %u.\n",p,time);
        } 
        else 
        { 
          deadline_missed++;
          run_prcs->wait = run_prcs->wait + run_prcs->p;
          fprintf(edf_log,"Process P%u missed deadline at time %u.\n",p,time);
          run_prcs->next_dl = run_prcs->next_dl + run_prcs->p;
        }
        run_prcs->remt = 0;
        run_pq.pop();
      }

      else 
      { // If process cannot be preempted
        if ((next_pq.top().p >= run_prcs->remt + time) && run_prcs->remt + time <= run_prcs->next_dl) 
        {
          run_prcs->next_dl = run_prcs->next_dl + run_prcs->p;
          run_prcs->wait = run_prcs->wait + Waitingtime_formula(run_prcs,time);
          time = time + run_prcs->remt;
          fprintf(edf_log,"Process P%u finishes execution at time %u.\n",p,time);
          run_prcs->remt = 0;
          run_pq.pop();
        }

        else if (next_pq.top().p < run_prcs->next_dl)
        { 
          run_prcs->remt = run_prcs->remt - (next_pq.top().p - time);
          time = next_pq.top().p;
          INPUT tmp = prcs[next_pq.top().pid - 1];
          // Process preempting condition for EDF
          if((tmp.remk!=0) && ((tmp.next_dl < run_prcs->next_dl) || 
             (tmp.next_dl == run_prcs->next_dl)) && (next_pq.top().pid < p))
          {
            run_prcs->chk_prempt = true;
            fprintf(edf_log,"Process P%u is preempted by Process P%u at time %u Remaining processing time: %u.\n"
                     ,p,next_pq.top().pid,time,run_prcs->remt);
          }
          
        }
      }
    }  
   }
   
  // Total number of processes execeuted in the system 
  int ttl_prcs = 0;
  for(int i=0;i<N;i++)
  { ttl_prcs = ttl_prcs + prcs[i].k;}

  double ttl_avg_wt = 0;
  for (int i = 0; i < N; i++) 
  {
    prcs[i].wait = prcs[i].wait / prcs[i].k;
    ttl_avg_wt   = ttl_avg_wt + prcs[i].wait;
  }

  ttl_avg_wt  = ttl_avg_wt / N;

  FILE* edf_stat = fopen("EDF-Stats.txt","w");

  fprintf(edf_stat,"Number of processes that came into the system: %u\n",ttl_prcs);
  fprintf(edf_stat,"Number of processes successfully completed: %u\n",ttl_prcs - deadline_missed);
  fprintf(edf_stat,"Number of processes missed deadline: %u\n",deadline_missed);
  for (int i = 0; i < N; i++)
  {
    fprintf(edf_stat,"Average Waiting time for Process%u: %.2lf\n",prcs[i].pid,prcs[i].wait);
  }

}

int main() 
{
  // Number of processes
  unsigned num_of_prcs;
  // Opening inp-params.txt file 
  FILE* fp;
  fp = fopen("inp-params.txt","r");
  fscanf(fp,"%u",&num_of_prcs);

  INPUT* prcs;
  prcs = (INPUT*)malloc(num_of_prcs*sizeof(INPUT));
  
  // Assigning values for the Input (process initial information)
  for(int i=0;i<num_of_prcs;i++)
  {
    fscanf(fp,"%u %u %u %u",&prcs[i].pid,&prcs[i].t,&prcs[i].p,&prcs[i].k);
    prcs[i].remt = 0;  // Updated as burst time in scheduler
    prcs[i].remk = prcs[i].k; // Remaining number of times 
    prcs[i].arrt = 0;         // Arrival time
    prcs[i].wait = 0;         // Waiting time
    prcs[i].chk_prempt = false;  // Preemption checker
  }

  //Closing inp-params.txt file 
  fclose(fp);
  
  // Executing Earliest Deadline First Scheduler simulator function
  EDF_Sheduler(num_of_prcs,prcs);
  
 free(prcs);
 return 0;
}
