#include "peerdownload.h"

extern unordered_map<string,vector<int>> filechunks_map;

struct FILEDATA{
	string name;
	string path;
	string hash;
	string size;
	string groupname;
	string serverpath;
	string username;
	vector<string> seeders;
	unordered_map<string,pair<string,string> > details;
};

struct ChunkData
{
	struct FILEDATA* pointer;
	int chunk_num;
	int seeder_num;
};

bool my_comp(const pair<int,vector<int>> &a,const pair<int,vector<int>> &b){
	if(a.second.size()<=b.second.size())
		return true;
	return false;
}

void populate(vector<vector<int> > &matrix,vector<pair<int,vector<int> > > v){
	int count=0;
	for(auto i:v){
		matrix[count][0]=i.first;
		for(auto j:i.second){
			// cout<<i.first<<" "<<j<<endl;
			matrix[count][j]=1;
		}
		++count;
	}
}

void Decide(vector<vector<int> > matrix,vector<int> &chunks){
	int n=matrix.size();
	int m=matrix[0].size();

	// cout<<n<<" "<<m<<endl;

	for(int j=1;j<m;++j){
		for(int i=0;i<n;++i){
			if(matrix[i][j]!=-1){
				int x=j-1;
				// cout<<i<<" "<<j<<" "<<x<<endl;
				if(chunks[x]==-1){
					chunks[x]=matrix[i][0];
				}
			}
		}
	}
}

vector<string> split2(string data,char delim){
	vector<string> v;
	string temp="";
	for(long long unsigned int i=0;i<data.size();++i){
		if(data[i]==delim){
			v.push_back(temp);
			temp="";
		}
		else{
			// const char x=data[i];
			// cout<<x<<endl;
			temp.append(data,i,1);
			// cout<<temp<<endl;
		}
	}
	v.push_back(temp);

	return v;
}

vector<pair<string,int> >Conv2(vector<string> split_vector){
	vector<pair<string,int> >tracker_data;
	// int n=split_vector.size();

	// for(unsigned int k=0;k<split_vector.size();++k)
	// 	cout<<k<<" "<<split_vector[k]<<endl;

	tracker_data.push_back(make_pair(split_vector[0],atoi(split_vector[1].c_str())));
	tracker_data.push_back(make_pair(split_vector[2],atoi(split_vector[3].c_str())));

	// cout<<"tracker_data"<<endl;

	// for(auto i:tracker_data)
	// 	cout<<i.first<<" "<<i.second<<endl;

	return tracker_data;
}

pair<string,int> CheckTracker2(vector<pair<string,int> > tracker_data){

	struct sockaddr_in remote_server;
	int sock;
	pair<string,int> server_final;

	for(auto i:tracker_data){
		// cout<<split_vector[0]<<endl;
		// cout<<split_vector[1]<<endl;

		if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
			perror("socket");
			// exit(-1);
		}

		remote_server.sin_family=AF_INET;
		remote_server.sin_port=htons(i.second);
		remote_server.sin_addr.s_addr=inet_addr(i.first.c_str());
		bzero(&remote_server.sin_zero,8);

		if(!((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1)){
			// perror("connect");
			// exit(-1);
			server_final=make_pair(i.first,i.second);
			// cout<<"Connected to "<<i.second<<endl;

			close(sock);

			return server_final;
		}
	}
	return server_final;
}

void print(std::vector<int> v){
	cout<<"########"<<endl;
	for(auto i:v){
		cout<<i<<" ";
	}
	cout<<endl;
}

pair<bool,vector<int> > CalcChunk(long long int nchunks,struct FILEDATA *filemeta){
	vector <int> seeders_chunk(nchunks,-1);
	vector<vector<int> > matrix(filemeta->seeders.size(),vector<int> (nchunks+1,-1));
	vector<pair<int,vector<int> > > v;
	bool flag=false;

	for(unsigned int i=0;i<filemeta->seeders.size();++i){
		string conn_ip=filemeta->details[filemeta->seeders[i]].first;
		int conn_port=atoi((filemeta->details[filemeta->seeders[i]].second).c_str());

		struct sockaddr_in remote_server;
		int sock;
		char output[MAX_SIZE1];

		if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
			perror("socket");
			exit(-1);
		}

		remote_server.sin_family=AF_INET;
		remote_server.sin_port=htons(conn_port);
		remote_server.sin_addr.s_addr=inet_addr(conn_ip.c_str());
		bzero(&remote_server.sin_zero,8);

		if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
			perror("connect");
			exit(-1);
		}

		string data="chunk_numbers "+filemeta->name;
		send(sock,data.c_str(),data.size(),0);
		int len=recv(sock,output,MAX_SIZE1,0);
		output[len]='\0';
		// cout<<i<<" "<<output<<endl;
		string output_string(output);
		std::vector<int> temp;

		if(output_string=="-1"){
			for(int i=0;i<nchunks;++i)
				temp.push_back(i+1);

			v.push_back(make_pair(i,temp));
		}
		else if(output_string=="-2"){
			v.push_back(make_pair(i,temp));
			flag=true;
		}
		else{
			auto split_vector=split2(output_string,' ');
			for(auto x:split_vector)
				temp.push_back(atoi(x.c_str())+1);

			v.push_back(make_pair(i,temp));
			flag=true;
		}
	}
	sort(v.begin(),v.end(),my_comp);
	populate(matrix,v);
	Decide(matrix,seeders_chunk);
	// print(seeders_chunk);

	auto ret=make_pair(flag,seeders_chunk);

	return ret;

}

string ServerPath(string filename){
	string tracker_data="",line="";

	ifstream infile;
	infile.open(filename,ios::in);

	while(getline(infile,line)){
		// cout<<line<<endl;
		// stringstream line_object(line);
		// line_object>>temp_ip;
		// line_object>>temp_port;
		// tracker_info.push_back(make_pair(temp_ip,temp_port));
		auto split_vector_temp=split2(line,' ');
		tracker_data+=split_vector_temp[0]+" "+split_vector_temp[1]+" ";
	}
	tracker_data=tracker_data.substr(0,tracker_data.size()-1);
	return tracker_data;
}

// void GetChunks(struct FILEDATA* filemeta){
// 	for(auto i:filemeta->details){
// 		struct sockaddr_in remote_server;
// 		int sock;
// 		char output[MAX_SIZE1];

// 		if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
// 			perror("socket");
// 			exit(-1);
// 		}

// 		remote_server.sin_family=AF_INET;
// 		remote_server.sin_port=htons(atoi(i.second.second.c_str()));
// 		remote_server.sin_addr.s_addr=inet_addr(i.second.first.c_str());
// 		bzero(&remote_server.sin_zero,8);

// 		if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
// 			perror("connect");
// 			exit(-1);
// 		}

// 		string data="Hello "+i.first;
// 		send(sock,data.c_str(),data.size(),0);
// 		int len=recv(sock,output,MAX_SIZE1,0);
// 		output[len]='\0';
// 		cout<<output<<endl;

// 		close(sock);
// 	}
// }

bool GetFileHashsmall(string filename,int nchunks,string fullhash){
	int max_limit=CHUNK_SIZE1;
	
	unsigned char result[2*SHA_DIGEST_LENGTH];
	unsigned char hash[SHA_DIGEST_LENGTH];

	FILE *f = fopen(filename.c_str(),"rb");
	long long int pos=nchunks*CHUNK_SIZE1;
	fseek(f,pos,SEEK_SET);

	// if(f==NULL){
	// 	return "";
	// }

	SHA_CTX mdContent;
	unsigned char data[max_limit+1];
	int bytes;
	// long long int file_size=0;
	// string final_hash="";

	bytes=fread(data, 1, max_limit, f);
	data[bytes]='\0';
	// file_size+=bytes;

	SHA1_Init(&mdContent);

	SHA1_Update(&mdContent, data, bytes);
	SHA1_Final(hash,&mdContent);
	for(int i=0; i < SHA_DIGEST_LENGTH;i++){
	  sprintf((char *)&(result[i*2]), "%02x",hash[i]);
	}
	// printf("%s\n",result);
	// cout<<bytes<<endl;
	string temp_hash(result,result+20);
	string x=fullhash.substr(nchunks*20,20);

	// cout<<temp_hash.size()<<endl;

	// final_hash=final_hash+temp_hash;

	// return final_hash;





	if(temp_hash==x)
		return true;
	else
		return false;
}


// void UpdateServer(struct FILEDATA* filemeta){
// 	auto split_vector=split2(filemeta->serverpath,' ');
// 	auto all_trackers=Conv2(split_vector);
// 	auto online_tracker=CheckTracker2(all_trackers);

// 	string data="add_file "+filemeta->groupname+" "+filemeta->name+" "+filemeta->path+" "+filemeta->username;

// 	struct sockaddr_in remote_server;
// 	int sock;

// 	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
// 		perror("socket");
// 		exit(-1);
// 	}

// 	remote_server.sin_family=AF_INET;
// 	remote_server.sin_port=htons(online_tracker.second);
// 	remote_server.sin_addr.s_addr=inet_addr(online_tracker.first.c_str());
// 	bzero(&remote_server.sin_zero,8);

// 	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
// 		perror("connect");
// 		exit(-1);
// 	}

// 	send(sock,data.c_str(),data.size(),0);
// 	close(sock);
// }

void UpdateServer(struct ChunkData* chunkmeta){
	auto split_vector=split2(chunkmeta->pointer->serverpath,' ');
	auto all_trackers=Conv2(split_vector);
	auto online_tracker=CheckTracker2(all_trackers);

	string data="add_file "+chunkmeta->pointer->groupname+" "+chunkmeta->pointer->name+" "+chunkmeta->pointer->path+" "+chunkmeta->pointer->username;

	// cout<<"Sending Update"<<endl;
	// cout<<data<<endl;

	struct sockaddr_in remote_server;
	int sock;

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(online_tracker.second);
	remote_server.sin_addr.s_addr=inet_addr(online_tracker.first.c_str());
	bzero(&remote_server.sin_zero,8);

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	send(sock,data.c_str(),data.size(),0);
	close(sock);
}

void UpdateServerComplete(struct FILEDATA* filemeta){
	auto split_vector=split2(filemeta->serverpath,' ');
	auto all_trackers=Conv2(split_vector);
	auto online_tracker=CheckTracker2(all_trackers);

	string data="complete "+filemeta->groupname+" "+filemeta->name+" "+filemeta->path+" "+filemeta->username;

	struct sockaddr_in remote_server;
	int sock;

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(online_tracker.second);
	remote_server.sin_addr.s_addr=inet_addr(online_tracker.first.c_str());
	bzero(&remote_server.sin_zero,8);

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	send(sock,data.c_str(),data.size(),0);
	close(sock);
}

void* DownloadKernel(void* pointer){
point:
	struct ChunkData* chunkmeta=(struct ChunkData *)pointer;

	int num=chunkmeta->seeder_num;
	auto seeders=chunkmeta->pointer->seeders;
	auto details=chunkmeta->pointer->details;
	string seeder_name=seeders[num];
	string conn_ip=details[seeders[num]].first;
	int conn_port=atoi((details[seeders[num]].second).c_str());

	// cout<<"Num is "<<num<<endl;
	// cout<<seeders[num]<<endl;
	// cout<<conn_ip<<endl;
	// cout<<conn_port<<endl;

	struct sockaddr_in remote_server;
	int sock;
	char output[SMALL_CHUNK_SIZE1];

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(conn_port);
	remote_server.sin_addr.s_addr=inet_addr(conn_ip.c_str());
	bzero(&remote_server.sin_zero,8);

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	string data="send_chunk "+chunkmeta->pointer->groupname+" "+chunkmeta->pointer->name+" "+to_string(chunkmeta->chunk_num)+" "+chunkmeta->pointer->serverpath+" "+seeder_name;
	send(sock,data.c_str(),data.size(),0);

	string filepath=chunkmeta->pointer->path;
	long long int pos=chunkmeta->chunk_num*CHUNK_SIZE1;
	long long int file_size=CHUNK_SIZE1,n;

	FILE * fs=fopen(filepath.c_str(),"rb+");
	fseek(fs,pos,SEEK_SET);

	
	while ((n=recv(sock,output,SMALL_CHUNK_SIZE1,0))>0&&file_size>0){
		fwrite(output,sizeof(char),n,fs);
		memset(output,'\0',SMALL_CHUNK_SIZE1);
		file_size = file_size-n;
	}

	fclose(fs);

	close(sock);

	if(chunkmeta->chunk_num==0){
		UpdateServer(chunkmeta);
		// cout<<"Done"<<endl;
	}

	if(!GetFileHashsmall(filepath,chunkmeta->chunk_num,chunkmeta->pointer->hash))
		goto point;

	filechunks_map[chunkmeta->pointer->name].push_back(chunkmeta->chunk_num);

	return NULL;
}

void GetChunks(struct FILEDATA* filemeta){
	long long int nchunks1=atoi((filemeta->size).c_str()),x;
	// cout<<"No. of chunks are "<<nchunks1<<endl;
	// cout<<"No. of chunks are "<<CHUNK_SIZE1<<endl;
	long long int nchunks=nchunks1/CHUNK_SIZE1;
	if(nchunks1%CHUNK_SIZE1)
		++nchunks;
	// cout<<"No. of chunks are "<<nchunks<<endl;
	pthread_t tid[1000];
	int counter=0;
	int nseeder=filemeta->seeders.size();
	int ncounter=0;


	vector<int>t;
	filechunks_map[filemeta->name]=t;

	auto decide=CalcChunk(nchunks,filemeta);

	for(long long int i=0;i<nchunks;++i){
		// pthread_t tid;

		// if(i%2)
		// 	x=1;
		// else x=0;
		if(!decide.first){
			x=ncounter;
			++ncounter;
			ncounter=ncounter%nseeder;
		}
		else{
			x=decide.second[i];
		}

		struct ChunkData* chunkmeta=new ChunkData;
		chunkmeta->pointer=filemeta;
		chunkmeta->chunk_num=i;
		chunkmeta->seeder_num=x;

		// cout<<chunkmeta->chunk_num<<" "<<chunkmeta->seeder_num<<endl;

		int pid=pthread_create(&tid[counter],NULL,DownloadKernel,(void *)chunkmeta);
		if(pid!=0){
			perror("thread failed");
			exit(-1);
			// cout<<"This has failed"<<endl;
		}
		++counter;
		counter=counter%1000;

		if(counter>50){
			for(int j=0;j<counter;++j){
				pthread_join(tid[j],NULL);
			}
			counter=0;
		}
	}

	for(long long int i=0;i<counter;++i){
		pthread_join(tid[i],NULL);
	}

}

string GetFileHash2(string filename){
	int max_limit=CHUNK_SIZE1;
	
	unsigned char result[2*SHA_DIGEST_LENGTH];
	unsigned char hash[SHA_DIGEST_LENGTH];

	FILE *f = fopen(filename.c_str(),"rb");

	if(f==NULL){
		return "";
	}

	SHA_CTX mdContent;
	unsigned char data[max_limit+1];
	int bytes;
	// long long int file_size=0;
	string final_hash="";

	while((bytes=fread(data, 1, max_limit, f))){
		data[bytes]='\0';
		// file_size+=bytes;

		SHA1_Init(&mdContent);

		SHA1_Update(&mdContent, data, bytes);
		SHA1_Final(hash,&mdContent);
		for(int i=0; i < SHA_DIGEST_LENGTH;i++){
		  sprintf((char *)&(result[i*2]), "%02x",hash[i]);
		}
		// printf("%s\n",result);
		// cout<<bytes<<endl;
		string temp_hash(result,result+20);

		// cout<<temp_hash.size()<<endl;

		final_hash=final_hash+temp_hash;
	}

	return final_hash;
}

void MatchSHA(struct FILEDATA *filemeta){
	string filepath=filemeta->path;
	string x=GetFileHash2(filepath);

	if(x==filemeta->hash)
		cout<<"Full file matched"<<endl;
	else
		cout<<"Not Matched"<<endl;

}

void DownloadFile(string groupname, string filename, string filepath,string username,string myip,string si,int sp,string serverpath){

	struct FILEDATA* filemeta=new FILEDATA;
	struct sockaddr_in remote_server;
	int sock;
	char output[MAX_SIZE1];

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(sp);
	remote_server.sin_addr.s_addr=inet_addr(si.c_str());
	bzero(&remote_server.sin_zero,8);

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	string data="give_sha "+groupname+" "+filename+" "+username;
	send(sock,data.c_str(),data.size(),0);
	int len=recv(sock,output,MAX_SIZE1,0);
	output[len]='\0';
	string filehash(output);

	if(strlen(output)==0){
		cout<<"Invalid request"<<endl;
		return;
	}
	// else
	// 	cout<<output<<endl;

	close(sock);

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	data="give_seeders "+groupname+" "+filename+" "+username;

	send(sock,data.c_str(),data.size(),0);
	len=recv(sock,output,MAX_SIZE1,0);
	output[len]='\0';

	close(sock);

	if(strlen(output)==0){
		cout<<"No seeders available"<<endl;
		return;
	}
	else{
		string output_string(output);
		auto split_vector=split2(output_string,' ');
		// for(auto i:split_vector)
			// cout<<i<<endl;
		string size=split_vector[split_vector.size()-1];
		split_vector.erase(split_vector.end()-1);
		// cout<<size<<endl;

		unordered_map<string,pair<string,string> > details;

		for(auto i:split_vector){
			// cout<<i<<endl;

			if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
				perror("socket");
				exit(-1);
			}

			if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
				perror("connect");
				exit(-1);
			}

			data="give_details "+i;

			send(sock,data.c_str(),data.size(),0);
			len=recv(sock,output,MAX_SIZE1,0);
			output[len]='\0';

			close(sock);

			string cred(output);
			auto split_vector2=split2(cred,' ');

			details[i]=make_pair(split_vector2[0],split_vector2[1]);
		}

		// for(auto j:details)
			// cout<<j.first<<" "<<j.second.first<<" "<<j.second.second<<endl;

		filemeta->name=filename;
		filemeta->path=filepath;
		filemeta->hash=filehash;
		filemeta->size=size;
		filemeta->seeders=split_vector;
		filemeta->details=details;
		filemeta->groupname=groupname;
		filemeta->username=username;
		filemeta->serverpath=ServerPath(serverpath);

		ofstream outfile(filepath,ios::out);
		// cout<<atoi(size.c_str())<<endl;
		for(long long int i=0;i<atoi(size.c_str());++i)
			outfile.write("5",1);
	}

	GetChunks(filemeta);
	// UpdateServer(filemeta);
	UpdateServerComplete(filemeta);
	MatchSHA(filemeta);
	// close(sock);

	// for(auto ix:filechunks_map){
	// 	cout<<ix.first<<" ";
	// 	for(auto j:ix.second)
	// 		cout<<j<<" ";
	// 	cout<<endl;
	// }

	filechunks_map.erase(filemeta->name);
	return;
}