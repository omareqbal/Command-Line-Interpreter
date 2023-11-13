//Implementation of a rudimentary command-line interpreter running on the Linux operating system

#include<bits/stdc++.h>
#include<stdlib.h>
#include<sys/types.h> 
#include<sys/wait.h>
#include<unistd.h> 
#include<time.h>
#include<string.h>
#include<fcntl.h>
using namespace std;

#define MAX 10 

int main()
  {
    char command[100];                         //Will contain the command to be taken as input
	vector<const char*> args,allargs;          //will be used to form the array of arguments to be passed into the execv function
	int i,pid,j,k,pchild;				   	   //i,j,k is for looping, pid,pchild for forking
	char *token;				   			   //to split the command between space to get various arguments
	char* const *a;							   //to form the argument for execv
	int infile,outfile;						   //for < and > type instructions
	vector<int> numpipes;					   //will store the position of pipes and its size will tell the number of pipes
	bool outred=false,inred=false;
				
    printf("Enter the command that you want to execute (\"quit\" to exit from the program): \n>> ");

    while(1)
          {
          	 	  cin.getline(command,100);			//Input the command
                  int p[MAX][2];                    //p[MAX][2] is the 2D array of MAX x 2 elements that can make at most MAX pipes
                  if(strcmp(command,"")==0) 
                  	{
                  		cout<<">> ";               //Empty command, simply continue
                  		continue;
                  	}
                  //Use strtok to split the command between spaces and add those tokens into the args vector
                  token=strtok(command," ");
                  while(token!=NULL)
                    {
                      allargs.push_back(token);
                      token = strtok(NULL," "); 
                    }
     
                  if(strcmp(allargs[0],"quit")==0) exit(0);	           //quit if quit is written

				  for(i=0;i<allargs.size();i++)
				  	{
				  		if(strcmp(allargs[i],"|")==0)  { numpipes.push_back(i); }         //Store the position of pipes first
				  	}
				  for(i=0;i<numpipes.size();i++)
				  	{
				  	  if(pipe(p[i])<0) 
						  {
						  	cout<<"Pipe creation failed"<<endl; exit(0);				//Create pipes
						  }
				  	}

				  //NOW SPLIT IT INTO TWO CASES. IF THERE IS AT LEAST ONE PIPE

				  if(numpipes.size()>0)		
				  		{
				  			//Implies there are at least 2 commands

				  			//First run the initial command with input redirection as STDIN or the file if specified

				  			  pchild=fork();
							  if(pchild<0) cout<<"Fork creation failed\n";              
								  		
							  else if(pchild==0)
								  {		
									vector<const char*> cmdargs;          //Make vector cmdargs to store the arguments of that file
									j=0;
									  		
									while(j<allargs.size()&&strcmp(allargs[j],"<")!=0&&strcmp(allargs[j],">")!=0&&strcmp(allargs[j],"|")!=0)
									   {
									     cmdargs.push_back(allargs[j]);		//Store the arguments till you encounter < or |
									     j++;
									   }

									while(strcmp(allargs[j],"|"))
								    {

								    	if(strcmp(allargs[j],"<")==0) 		  //If you encounter <, then change the input redirection
								          {
								             infile=open(allargs[j+1],O_RDONLY);
									   	     if(infile==-1) 
										    	{
										    		cout<<"Error: File not Found"<<endl;
										    		exit(2);
										    	}
								             close(0);
						          		     dup(infile);
						          		     inred=true;
								          }

								    	if(strcmp(allargs[j],">")==0)
								    	{
								    		outfile=open(allargs[j+1],O_WRONLY| O_CREAT | O_TRUNC,0666);
										     if(outfile==-1) 
											    	{
											    		cout<<"Error: File not Found"<<endl;
											    		exit(2);
											    	}
										     close(1);
								  		     dup(outfile);						//change the output redirection if > is found
								  		     outred=true;
								    	}

								    	j++;
								    }

								    if(outred==false)
								    {
								    	close(1);
								    	dup(p[0][1]);
								    }

								    cmdargs.push_back(NULL);
									a=(char* const *)&cmdargs[0];        
								    if(execvp(a[0],a)<0) 		         //run the process in execvp process
								      	  	{
								      	  	   cout<<"Cannot execute command"<<endl;
								      	  	   exit(2);
								      	  	}   
								  }
							 	else {	      
										wait(NULL);                     //return to parent process
										
										close(p[0][1]);					//all the output of the process has been written to p[0][1], now close that end of pipe
							     	 }

							    //NOW IN FOR LOOP, iteratively create a process with input redirection as read end of previous pipe
							    //and output redirection as write end of current pipe

							    for(i=1;i<numpipes.size();i++)
								  	{
								  		pchild=fork();
								  		if(pchild<0) cout<<"Fork creation failed\n";     //fork a child
								  		
								  		else if(pchild==0)
								  		{		
									  		vector<const char*> cmdargs;
									  		j=numpipes[i-1]+1;
									  		
									  		close(0);					//change the input and output redirections
											dup(p[i-1][0]);
											close(1);
											dup(p[i][1]);

									  		while(j<allargs.size()&&strcmp(allargs[j],"<")!=0&&strcmp(allargs[j],">")!=0&&strcmp(allargs[j],"|")!=0)
											  	{  
											  		cmdargs.push_back(allargs[j]);       //get all the arguments in cmdargs
											  		j++;
											  	}
											while(strcmp(allargs[j],"|"))
											    {

											    	if(strcmp(allargs[j],"<")==0) 		  //If you encounter <, then change the input redirection
											          {
											             infile=open(allargs[j+1],O_RDONLY);
												   	     if(infile==-1) 
													    	{
													    		cout<<"Error: File not Found"<<endl;
													    		exit(2);
													    	}
											             close(0);
									          		     dup(infile);
											          }

											    	if(strcmp(allargs[j],">")==0)
											    	{
											    		outfile=open(allargs[j+1],O_WRONLY| O_CREAT | O_TRUNC,0666);
													     if(outfile==-1) 
														    	{
														    		cout<<"Error: File not Found"<<endl;
														    		exit(2);
														    	}
													     close(1);
											  		     dup(outfile);						//change the output redirection if > is found
											    	}
											    	j++;
											    }

											cmdargs.push_back(NULL);
											a=(char* const *)&cmdargs[0];        
										    if(execvp(a[0],a)<0) 		    //run the process in execvp process
										      	  	{
										      	  	   cout<<"Cannot execute command"<<endl;
										      	  	   exit(2);
										      	  	} 
										}
										else 
											{
												wait(NULL);	
												close(p[i][1]);       //close the write end of current pipe once everything is written
											}	  	
								  	}

								  //NOW COMES THE LAST PROCESS with input redirection as read end of last pipe and output redirection
								  //as either STDOUT or the file specified using >

								  pchild=fork();                                  //fork a child
								  if(pchild<0) cout<<"Fork creation failed\n";
									  		
								  else if(pchild==0)
									  {	
									  	close(0);
										dup(p[numpipes.size()-1][0]);			   //change the input redirection 
										vector<const char*> cmdargs;
										j=numpipes[numpipes.size()-1]+1;           //go to the element after the last pipe
										  		
										while(j<allargs.size()&&strcmp(allargs[j],"<")!=0&&strcmp(allargs[j],">")!=0&&strcmp(allargs[j],"&")!=0&&strcmp(allargs[j],"|")!=0)
										   {
										     cmdargs.push_back(allargs[j]);         //push the arguments here 
										     j++;
										   }
										
										while(j<allargs.size())
											    {

											    	if(strcmp(allargs[j],"<")==0) 		  //If you encounter <, then change the input redirection
											          {
											             infile=open(allargs[j+1],O_RDONLY);
												   	     if(infile==-1) 
													    	{
													    		cout<<"Error: File not Found"<<endl;
													    		exit(2);
													    	}
											             close(0);
									          		     dup(infile);
											          }

											    	if(strcmp(allargs[j],">")==0)
											    	{
											    		outfile=open(allargs[j+1],O_WRONLY| O_CREAT | O_TRUNC,0666);
													     if(outfile==-1) 
														    	{
														    		cout<<"Error: File not Found"<<endl;
														    		exit(2);
														    	}
													     close(1);
											  		     dup(outfile);						//change the output redirection if > is found
											    	}
											    	j++;
											    }
											
										cmdargs.push_back(NULL);
										a=(char* const *)&cmdargs[0];        
									    if(execvp(a[0],a)<0) 		    //run the process in execvp process
									      	  	{
									      	  	   cout<<"Cannot execute command"<<endl;
									      	  	   exit(2);
									      	  	} 
									  }
								 else
									  {
											if(strcmp(allargs[allargs.size()-1],"&")!=0) wait(NULL);   //wait if you don't get &
										  	allargs.clear();
										  	numpipes.clear();
											cout<<"\n>> ";
									  }
				  		}



				  // IF NO PIPE WAS FOUND i.e numpipes.size()==0
				  else {
		                  pid=fork();						//fork a child process
				          if(pid<0) cout<<"Fork creation failed\n";	  
				          
				          else if(pid==0) 
				            	{ 
				            	  i=0;
								  while(i<allargs.size()&&strcmp(allargs[i],"<")!=0&&strcmp(allargs[i],">")!=0&&strcmp(allargs[i],"&")!=0&&strcmp(allargs[i],"|")!=0)
								  	{
								  		args.push_back(allargs[i]);      //Push all the arguments till you get a > or < or &
								  		i++;
								  	}

								  for(i=0;i<allargs.size();i++)
								  	{	
									  	if(strcmp(allargs[i],"<")==0)			//In case of <, change the input file
									  		{
									  		    infile=open(allargs[i+1],O_RDONLY);
											    if(infile==-1) 
											    	{
											    		cout<<"Error: File not Found"<<endl;
											    		exit(2);
											    	}
									  		    close(0);
								  		    	    dup(infile);    
								  			}
								  	  	else if(strcmp(allargs[i],">")==0)		//In case of >, change the output file
									  		{
									  		    outfile=open(allargs[i+1],O_WRONLY | O_CREAT | O_TRUNC,0666); 
										  	    close(1);
									  		    dup(outfile);
									  		}
						          	}
				              	  //push NULL at the end of the args vector (standard array argument for execvp has NULL at end)
								  args.push_back(NULL);
								  a=(char* const *)&args[0];        //convert the vector into char* const* which is the type of argument for execvp
						              	  if(execvp(a[0],a)<0) 		    //run the process in execvp process
						              	  	{
						              	  	   cout<<"Cannot execute command"<<endl;
						              	  	   exit(2);
						              	  	}              
				                } 
						  else
							{
								if(strcmp(allargs[allargs.size()-1],"&")!=0) wait(NULL);      //Wait if & is found
						        args.clear();                    //clear the args
						        allargs.clear();
						        numpipes.clear();
								cout<<"\n>> ";
							}
					}
			  }
	return 0;
}