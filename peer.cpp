#include<stdio.h>
#include<string.h>
#include<cstdlib>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<fstream>
#include<iostream>
#include "boost/lexical_cast.hpp"
#include "boost/lexical_cast.hpp"
#include <boost/algorithm/string.hpp> 


using namespace std;

string logged_user;
int login_flag ;

struct download
{
	int port;
	int start;
	int size;
	string file_name;
	string des_path;
};

struct argument
{	
	int socket;
	int my_port;
	string command;
};

void *handler(void *socket_desc)
{
	int sock = *(int *)socket_desc;
	int read_size;

	char msg[2000];
	memset(msg ,'\0' ,2000);

	int start ,csize ;

	cout<<"handler Called"<<endl;
	
	recv(sock ,msg ,2000,0);

	string data = string(msg);

	vector<string> res;
	boost::split(res ,data ,boost::is_any_of(","));

	start = boost::lexical_cast<int>(res[1]);
	csize = boost::lexical_cast<int>(res[2]);
	string file = res[0]; 

	cout<<"File Name Received :"<<file<<" "<<start<<" "<<csize<<endl;

	FILE *fp = fopen(file.c_str() ,"r+");

	int n;

	if( fp == NULL )
	{
		cout<<"Not Opened "<<endl;
	}

	fseek(fp ,start ,SEEK_SET);

	int count  = 0;
	int null_count = 0;
	
	while( csize > 0 )
	{
		count++;
		char buf;
		memset(&buf ,'\0' ,1);

		n = fread( &buf ,sizeof(char) ,1 ,fp) ;
		
		if( buf == '\0')
		{
			null_count++;
		}	
		//cout<<buf<<endl;

		send(sock,&buf,sizeof(buf),0);
		cout<<buf;
		
		csize = csize-n;
	}

	cout<<"Data sent : "<<count<<" NULL Count :"<<null_count<<" "<<endl;
	fclose(fp);
	close(sock);
}

void *server(void *args)
{
	int my_port = *(int *)args;

	int server_sock = socket(AF_INET ,SOCK_STREAM ,0);
	if( server_sock == -1)
	{
		cout<<"Server Socket Not Created"<<endl;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( my_port );

	int bind_status = bind(server_sock ,(struct sockaddr *)&server_addr ,sizeof(server_addr));
	if( bind_status <  0)
	{
		cout<<"Server Bind Failed "<<endl;
	}

	listen(server_sock ,10);

	int c = sizeof(server_addr);


	pthread_t req[100000];	

	int req_sock[100000];
	int i=0;
	
	while(1)
	{
		cout<<"waiting For request"<<endl;
		req_sock[i] = accept( server_sock ,(struct sockaddr *)&server_addr ,(socklen_t *)&c);
		pthread_create(&req[i] ,NULL ,handler ,(void *)&req_sock[i]);
		i++;
	}

	cout<<"Server Exits"<<endl;
}

void *threaded_download(void *args)
{
	struct download *dt = (struct download *)args;

	int peer_port = dt->port;
	int start = dt->start;
	int size = dt->size;
	string file = dt->file_name;
	string des_file = dt->des_path;

	string data = file +","+ boost::lexical_cast<string>(start) + "," + boost::lexical_cast<string>(size) ;

	cout<<data<<endl;

	int client_sock = socket(AF_INET ,SOCK_STREAM ,0);

	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(peer_port);
	client_addr.sin_addr.s_addr = INADDR_ANY;

	int check ;
	if( (check  = connect(client_sock ,(struct sockaddr *)&client_addr ,sizeof(client_addr) ) ) <  0)
	{
		cout<<"Connection Fail "<<endl;
		exit(1);
	}

	send( client_sock ,data.c_str() ,data.size() ,0);

	int n;

	FILE *fp  =fopen(des_file.c_str(),"r+");

	fseek(fp,start ,SEEK_SET);

	cout<<"Getting  Data "<<endl;

	int count =  0;
	int null_count = 0;
	
	while ( size>0 )
	{
		count++;

		char buf;	
		memset ( &buf , '\0', 1);
		
		n = recv( client_sock ,&buf ,1, 0)  ;
		
		if( buf == '\0')
		{
			null_count++;
		}

		//cout<<buff;
		fwrite(&buf , sizeof(char), 1, fp);
		
		size = size - n;
	}

	cout<<endl<<"Chunk at "<<start<<" Downloaded with download size :"<<count<<" Null count : "<<null_count<<endl;
	return NULL;
}

void download_file(vector<string> result ,string comm,int sock)
{
	//command : download_file grp_id file_name destination_path

	string des_path = result[3];
	send(sock ,comm.c_str(),comm.size() ,0);

	int file_size;
	recv(sock, &file_size ,sizeof(file_size) ,0);
	cout<<"File size : "<<file_size<<endl;

	int n;
	recv(sock, &n ,sizeof(n) ,0);
	cout<<"Chunks :"<<n<<endl;

	vector<vector<string>> data;

	int flag = 0;
	recv(sock ,&flag ,sizeof(flag),0);
	cout<<"flag :"<<flag<<endl;
	cout<<"reading File"<<endl;

	ifstream in;
	in.open("temp.txt");

	string x;

	while(getline(in ,x))
	{
		vector<string> result ;
		boost::split(result, x, boost::is_any_of(" "));

		data.push_back(result);
	}

	pthread_t download_threads[data.size()];
	struct download temp[data.size()];

	ofstream out;
	out.open(des_path);

	for(int i=0 ;i<=file_size ;i++)
	{
		out<<"\0";
	}
	out.close();


	for(int i=0 ;i<data.size() ;i++)
	{
		temp[i].port = boost::lexical_cast<int>(data[i][0]);
		temp[i].start = boost::lexical_cast<int>(data[i][1]);
		temp[i].size = boost::lexical_cast<int>(data[i][2]);
		temp[i].file_name = result[2];
		temp[i].des_path = des_path;

		pthread_create(&download_threads[i] ,NULL ,threaded_download ,(void *)&temp[i]);
	}

	for(int i=0 ;i<data.size() ;i++)
	{
		pthread_join(download_threads[i],NULL);
	}

	cout<<"Downloading Done"<<endl;

}

void *command_handler(void *args)
{
	struct argument *n = (struct argument *)args;

	int sock = n->socket;
	int my_port = n->my_port;
	string comm = n->command;


	vector<string> result; 
    boost::split(result, comm, boost::is_any_of(" ")); 

	if( (result[0].compare("create_user") ) == 0 )
	{
		send(sock ,comm.c_str() ,comm.size() ,0);
	}
	else if( (result[0].compare("login") )==0 )
	{
		if( login_flag == 0 )
		{
			logged_user = result[1];
			login_flag = 1;

			//login handling through tracker Remaining

			string temp = boost::lexical_cast<string>(my_port);
			comm = comm + " " + temp;
			send(sock ,comm.c_str() ,comm.size() ,0);
		}
		else
		{
			cout<<"Other User Already Logged In .Login Through Other Terminal"<<endl;
		}
	}
	else if( (result[0].compare("create_group")) == 0 )
	{
		if( login_flag == 1 )
		{
			comm = comm +" "+logged_user;
			send(sock ,comm.c_str() ,comm.size() ,0);	
		}
		else
		{
			cout<<"Please Login First"<<endl;
		}
	}
	else if( (result[0].compare("join_group")) == 0 )
	{
		if( login_flag == 1 )
		{
			comm = comm +" "+logged_user;
			send(sock,comm.c_str() ,comm.size() ,0);
		}
		else
		{
			cout<<"Please Login First"<<endl;
		}
	}
	else if( (result[0].compare("leave_group")) == 0 )
	{
		if( login_flag == 1)
		{
			comm = comm + " " + logged_user;
			send(sock ,comm.c_str() ,comm.size() ,0);
		}
		else
		{
			cout<<"PLease Login First"<<endl;
		}
	}
	else if( (result[0].compare("list_groups")) == 0 )
	{
		if( login_flag == 1)
		{
			send(sock ,comm.c_str() ,comm.size() ,0);
		}
		else
		{
			cout<<"Please Login First"<<endl;
		}
	}
	else if( (result[0].compare("logout")) == 0 )
	{
		if( login_flag == 1)
		{
			comm = comm + " "+logged_user;
			send(sock,comm.c_str() ,comm.size() ,0);

			logged_user = "";
			login_flag == 0;
		}
		else
		{
			cout<<"Please Login First "<<endl;
		}
	}
	else if( (result[0].compare("upload_file")) == 0)
	{
		if( login_flag == 1 )
		{
			FILE *fp = fopen ( result[1].c_str() , "rb" );

			if( fp == NULL )
			{
				cout<<"Invalid Path "<<endl;
				return NULL;
			}

			fseek ( fp , 0 , SEEK_END);
  			int size = ftell ( fp );
  			rewind (fp);

  			string fs = boost::lexical_cast<string>(size);

  			comm = comm + " " + logged_user + " " + fs;

  			send(sock,comm.c_str() ,comm.size() ,0);
		}
	}
	else if( (result[0].compare("list_files")) == 0 )
	{
		if( login_flag == 1 )
		{
			send(sock,comm.c_str(),comm.size() ,0);
		}
	}
	else if( (result[0].compare("download_file")) == 0 )
	{
		if( login_flag == 1)
		{
			download_file(result ,comm ,sock); 
		}
	}
}

void command_handler_function(string comm ,int peer_port ,int my_port)
{
	int client_sock = socket(AF_INET ,SOCK_STREAM ,0);

	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(peer_port);
	client_addr.sin_addr.s_addr = INADDR_ANY;

	int check ;
	if( (check  = connect(client_sock ,(struct sockaddr *)&client_addr ,sizeof(client_addr) ) ) <  0)
	{
		cout<<"Connection Fail "<<endl;
		exit(1);
	}

	pthread_t command_thread;

	vector<string> result; 
    boost::split(result, comm, boost::is_any_of(" ")); 

    struct argument args;
    args.socket = client_sock;
    args.command = comm;
    args.my_port = my_port;

	pthread_create(&command_thread ,NULL ,command_handler ,(void *)&args);

	pthread_join(command_thread ,NULL);

	return ;
}

int main(int argc ,char **argv)
{
	logged_user = "";
	login_flag = 0;

	vector<string> res;
	boost::split(res ,argv[1],boost::is_any_of(":"));

	int my_port = boost::lexical_cast<int>(res[1]);

	pthread_t ser;
	pthread_create(&ser ,NULL ,server ,(void *)&my_port);

	ifstream in;
	in.open(argv[2]);

	string pt;
	in>>pt;

	int peer_port = boost::lexical_cast<int>(pt);

	while(1)
	{	
		cout<<endl<<"Enter Command :"<<endl;
		string comm;
		getline(cin,comm);

		command_handler_function(comm ,peer_port ,my_port);
	}

	
	cout<<"Exit"<<endl;
	return 0;
}