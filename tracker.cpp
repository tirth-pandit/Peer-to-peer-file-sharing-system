#include<iostream>
#include<stdio.h>
#include<pthread.h>
#include <bits/stdc++.h> 
#include <fstream>
#include "boost/lexical_cast.hpp"
#include <boost/algorithm/string.hpp> 
#include<string.h>
#include<cstdlib>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "boost/lexical_cast.hpp"

using namespace std;

unordered_map<string ,string> users;
unordered_map<string ,int> logged_users;
multimap<string ,string> user_groups;
unordered_map<string ,string> grp_owners;
vector<vector<string>> file_info;

#define CHUNK_SIZE 40000

void read_users()
{
	ifstream in;

	in.open("user.txt");

	if( in.fail())
	{
		return ;
	}

	string x;

	while (getline(in,x)) 
	{
		
        vector<string> result; 
    	boost::split(result, x, boost::is_any_of(" "));

    	users[result[0]] = result[1]; 
    }

    in.close();
}

void read_logged_user()
{
	ifstream in;

	in.open("logged_user.txt" );

	if( in.fail())
	{
		return;
	}

	string x;

	while (getline(in,x)) 
	{
		vector<string> result; 
    	boost::split(result, x, boost::is_any_of(" "));

    	logged_users[result[0]] = boost::lexical_cast<int>(result[1]); 
    }

    in.close();
}

void read_group_owners()
{
	ifstream in;
	in.open("grp_owner.txt");

	if( in.fail())
	{
		return ;
	}

	string x;

	while(getline(in ,x))
	{
		vector<string> result;
		boost::split(result, x, boost::is_any_of(" "));

		grp_owners[result[0]] = result[1]; 
	}

	in.close();
}

void read_user_group()
{
	ifstream in;
	in.open("user_groups.txt");

	if( in.fail())
	{
		return ;
	}

	string x;

	while(getline(in ,x))
	{
		vector<string> result;
		boost::split(result, x, boost::is_any_of(" "));

		user_groups.insert(make_pair(result[0],result[1]));
	}

	in.close();
}

void read_uploded_files()
{
	ifstream in;
	in.open("file_info.txt");

	if( in.fail() )
	{
		return;
	}

	string x;

	while( getline(in ,x) )
	{
		vector<string> temp ;
		boost::split(temp ,x ,boost::is_any_of(" "));

		file_info.push_back(temp);
	}

	in.close();
}

void create_user(vector<string> result )
{
	
	users[result[1]] = result[2];

    ofstream out;

    out.open("user.txt",ios::app);

    out<<result[1]<<" "<<result[2]<<endl;

    out.close();
}

int login(vector<string> result)
{
	string uid = result[1];
	string pass = result[2];
	int port = boost::lexical_cast<int>(result[3]);

	if( users.find(uid) == users.end())
	{
		return 0; //user Not found
	}
	else if(  (pass.compare( (users[uid] ))) == 0 )
	{
		if(logged_users.find(uid) != logged_users.end())
		{
			return 2; //All ready logged in
		}

		logged_users[uid] = port;

		ofstream out;
		out.open("logged_user.txt");

		unordered_map<string ,int>::iterator it;

		for(it=logged_users.begin() ;it!=logged_users.end() ;it++)
		{
			out<<it->first<<" "<<it->second<<endl;
		}

		out.close();

		return 1; //login succes
	}
	else
	{
		return -1; //pass not matching
	}
}

int create_group(vector<string> result)
{
	string grp = result[1];
	string usr = result[2];

	user_groups.insert(make_pair(usr,grp));
	grp_owners[grp] = usr;

	ofstream out;
	out.open("grp_owner.txt");

	unordered_map<string ,string>::iterator it;
	for(it=grp_owners.begin() ;it!=grp_owners.end() ;it++)
	{
		out<<it->first<<" "<<it->second<<endl;
	}

	out.close();

	out.open("user_groups.txt");

	multimap<string ,string>::iterator itr;
	for(itr=user_groups.begin() ;itr!=user_groups.end() ;itr++)
	{
		out<<itr->first<<" "<<itr->second<<endl;
	}

	return 0;
}

int join_group(vector<string> result)
{
	string grp = result[1];
	string usr = result[2];

	user_groups.insert(make_pair(usr,grp));

	ofstream out;
	out.open("user_groups.txt");

	multimap<string ,string>::iterator itr;
	for(itr=user_groups.begin() ;itr!=user_groups.end() ;itr++)
	{
		out<<itr->first<<" "<<itr->second<<endl;
	}
	return 0;
}

int leave_group(vector<string> result)
{
	string grp = result[1];
	string usr = result[2];

	user_groups.erase(usr);

	ofstream out;
	out.open("user_groups.txt");

	multimap<string ,string>::iterator itr;
	for(itr=user_groups.begin() ;itr!=user_groups.end() ;itr++)
	{
		out<<itr->first<<" "<<itr->second<<endl;
	}

	return 0;
}

void list_groups()
{
	set<string> s;

	multimap<string ,string>::iterator i;

	for(i=user_groups.begin() ;i!=user_groups.end() ;i++)
	{
		s.insert( i->second );
	}

	set<string>::iterator it;

	for(it=s.begin() ;it!=s.end() ;it++)
	{
		string temp = *it;

		cout<< "Group ID : "<<*it<<endl;

		multimap<string ,string>::iterator itr;

		for(itr=user_groups.begin() ;itr!=user_groups.end() ;itr++)
		{
			string t = itr->second;

			if( t.compare(temp) == 0)
			{
				cout<<itr->first<<" ";
			}
		}
		cout<<endl;
	}
}

int logout(vector<string> result)
{
	string user = result[1];

	logged_users.erase(user);

	ofstream out;
	
	out.open("logged_user.txt");

	unordered_map<string ,int>::iterator it;

	for(it=logged_users.begin() ;it!=logged_users.end() ;it++)
	{
		out<<it->first<<" "<<it->second<<endl;
	}

	out.close();
}

int upload_file(vector<string> result)
{	
	//Result = command : FilePath : GrpID : UserID : FileSize 

	vector<string> file_path ;
	boost::split(file_path, result[1], boost::is_any_of("/"));

	string file_name = file_path[file_path.size()-1];

	vector<string> temp;
	temp.push_back(file_name);//FileName
	temp.push_back(result[1]);//FilePath
	temp.push_back(result[2]);//GrpId
	temp.push_back(result[3]);//UserID
	temp.push_back(result[4]);//FileSize

	file_info.push_back(temp);
	
	ofstream out;
	out.open("file_info.txt");

	for(int i=0 ;i<file_info.size() ;i++)
	{
		out<<file_info[i][0]<<" "<<file_info[i][1]<<" "<<file_info[i][2]<<" "<<file_info[i][3]<<" "<<file_info[i][4]<<endl;
	}	

	out.close();
}


void *handler(void *args)
{
	int sock = *(int *)args;

	char comm[1000];

	recv(sock ,comm ,1000,0);

	string command = string(comm);

	vector<string> result; 
    boost::split(result, command, boost::is_any_of(" ")); 

    cout<<command<<endl;

	if( (result[0].compare("create_user") )== 0 )
	{
		create_user(result);
		cout<<"User Created"<<endl;
	}
	else if( (result[0].compare("login") ) == 0 )
	{
		int sta = login(result);
		
		if( sta == 1 )
		{
			cout<<"Login Successful "<<endl;
		}
		else if( sta == -1)
		{
			cout<<"Incorrect Password "<<endl; 
		}
		else if( sta == 2 )
		{
			cout<<"Already Logged in"<<endl;
		}
		else
		{
			cout<<"User Not Created "<<endl;
		}
	}
	else if( (result[0].compare("create_group")) == 0 )
	{
		create_group(result);
		cout<<"Group Created"<<endl;
	}
	else if( (result[0].compare("join_group")) == 0 )
	{
		join_group(result);
		cout<<"Group Joined"<<endl;
	}
	else if( (result[0].compare("leave_group")) == 0 )
	{
		leave_group(result);
		cout<<"Group Leaved"<<endl;
	}
	else if( (result[0].compare("list_groups")) == 0 )
	{
		list_groups();
	}
	else if( (result[0].compare("logout")) == 0 )
	{
		logout(result);
	}
	else if( (result[0].compare("upload_file")) == 0 )
	{
		upload_file(result);
		cout<<"File Uploded"<<endl;
	}
	else if( (result[0].compare("list_files")) == 0 )
	{
		cout<<"Files in Group "<<result[1]<<endl;

		for(int i=0 ;i<file_info.size() ;i++)
		{
			string temp = file_info[i][2];

			if( (temp.compare(result[1]) ) == 0 )
			{
				cout<<"\t\t"<<file_info[i][0]<<endl;
			} 
		}
	}
	else if( (result[0].compare("download_file"))==0 )
	{
		//download_file grp_id file_name destination_path
		
		string grp = result[1];
		string file_name = result[2];

		vector<vector<string>> download_info;

		for(int i=0 ;i<file_info.size() ;i++)
		{
			string name = file_info[i][0];
			string gp = file_info[i][2];

			vector<string> temp;

			if( ((name.compare(file_name))==0) && ((gp.compare(grp))==0) )
			{
				temp.push_back(file_name); //file_name
				temp.push_back(file_info[i][1]); //file path
				
				string usr = file_info[i][3];
				string port = boost::lexical_cast<string>( logged_users[usr]);
				temp.push_back(port);// port

				temp.push_back(file_info[i][4]); //file size

				download_info.push_back(temp);
			}
		}

		int size = boost::lexical_cast<int>(download_info[0][3]);
		cout<<"File Size :"<<size<<" ";

		int chunks = size/CHUNK_SIZE;
		cout<<"No of Chunks : "<<chunks<<" ";

		vector<vector<int>> chunk_data;

		int peer_no = download_info.size();

		int s = 0;
		int i=0;
		for(i=0 ;i<chunks ;i++)
		{
			vector<int> temp;
			int index = i%peer_no;

			int port = boost::lexical_cast<int>(download_info[index][2]);
			int start = s;
			s += CHUNK_SIZE;

			temp.push_back(port);
			temp.push_back(start);
			temp.push_back(CHUNK_SIZE);

			chunk_data.push_back(temp);
		}

		int index = i%peer_no;
		int start = s;
		int cs = size - CHUNK_SIZE*chunks;

		if( cs != 0 )
		{
			vector<int> temp;
			int port = boost::lexical_cast<int>(download_info[index][2]);
			temp.push_back(port);
			temp.push_back(start);
			temp.push_back(cs);

			chunk_data.push_back(temp);
		}

		int data_size = chunk_data.size();

		send(sock ,&size ,sizeof(size) ,0);
		send(sock ,&data_size ,sizeof(data_size) ,0);	

		ofstream out;
		out.open("temp.txt");

		for(int i=0 ;i<chunk_data.size() ;i++)
		{
			string res = boost::lexical_cast<string>(chunk_data[i][0]) + " " + boost::lexical_cast<string>(chunk_data[i][1]) + " " + boost::lexical_cast<string>(chunk_data[i][2]) ;
			out<<res<<endl;
		}
		
		int flag = 1;
		send(sock ,&flag ,sizeof(flag) ,0);
	}
}

void *fun(void *args)
{
	int tracker_port = *(int *)args;

	int sock = socket(AF_INET ,SOCK_STREAM ,0);
	if( sock == -1 )
	{
		cout<<"Socket Not Created"<<endl;
	}

	struct sockaddr_in tracker;
	tracker.sin_family = AF_INET;
	tracker.sin_addr.s_addr = INADDR_ANY;
	tracker.sin_port = htons( tracker_port );
	
	int t = bind(sock,(struct sockaddr *)&tracker ,sizeof(tracker));
	if( t<0 )
	{
		cout<<"Bind Fail"<<endl;
	}

	listen(sock,10);

	int c=sizeof( struct sockaddr_in);

	pthread_t thread_id[100000];
	int sockets[100000];

	int i=0;

	while(1)
	{
		sockets[i] = accept( sock, (struct sockaddr *)&tracker, (socklen_t*)&c);
		cout<<endl<<"New Connection Accepted"<<endl;

		int p = pthread_create( &thread_id[i] ,NULL ,handler ,(void *)&sockets[i]);
		
		if( p < 0)
		{
			cout<<"Thread Not Created "<<endl;
			return NULL;
		}

		i++;
	}
}

main(int argc ,char **argv)
{
	read_users();
	read_logged_user();
	read_group_owners();
	read_user_group();
	read_uploded_files();

	int tracker_port = boost::lexical_cast<int>(argv[1]);
	pthread_t tracker;

	pthread_create(&tracker ,NULL ,fun ,(void *)&tracker_port);

	cout<<"Tracker Started "<<endl;
	string s;

	cout<<"Enter QUIT to close Tracker"<<endl;

	while(1)
	{
		cin>>s;

		if( s == "QUIT");
		{
			break;
		}
	}

	pthread_exit(NULL);

}