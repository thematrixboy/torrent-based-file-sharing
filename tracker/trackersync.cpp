#include "trackersync.h"

// pair<string,string> OtherServerDetails(int mysequence_i){
// 	int target_seq;
// 	if(mysequence_i==0){
// 		target_seq=1;
// 	}
// 	else
// 		target_seq=0;





// }

int GetTracker2(int mysequence_i){
	if(mysequence_i)
		return 0;

	return 1;
}

bool IsOnline(int target_seq,vector<pair<string,string>> tracker_info){
	struct sockaddr_in remote_server;
	int sock;

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		// exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(atoi(tracker_info[target_seq].second.c_str()));
	remote_server.sin_addr.s_addr=inet_addr(tracker_info[target_seq].first.c_str());
	bzero(&remote_server.sin_zero,8);

	if(!((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1)){
		close(sock);
		return true;
	}

	return false;
}


void Sync(string filename,int mysequence_i,vector<pair<string,string>> tracker_info){
	cout<<"Sync Send "<<filename<<" "<<mysequence_i<<endl;
	int target_seq=GetTracker2(mysequence_i);
	int bytes;
	FILE *f = fopen(filename.c_str(),"rb");
	
	if(!IsOnline(target_seq,tracker_info))
		return;

	struct sockaddr_in remote_server;
	int sock;
	char output[MAX_SIZE+1];

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(atoi(tracker_info[target_seq].second.c_str()));
	remote_server.sin_addr.s_addr=inet_addr(tracker_info[target_seq].first.c_str());
	bzero(&remote_server.sin_zero,8);

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	string initial="Sync "+filename;
	send(sock,initial.c_str(),initial.size(),0);
	int len=recv(sock,output,MAX_SIZE,0);

	while((bytes=fread(output,1,MAX_SIZE, f))){
		output[bytes]='\0';

		send(sock,output,bytes,0);
		len=recv(sock,output,MAX_SIZE,0);
		output[len]='\0';
	}

	string end="Sync Over";
	send(sock,end.c_str(),end.size(),0);
	close(sock);

}

void SyncAll(int mysequence_i,vector<pair<string,string>> tracker_info){
	int target_seq=GetTracker2(mysequence_i);

	if(!IsOnline(target_seq,tracker_info))
		return;

	struct sockaddr_in remote_server;
	int sock;
	// char output[MAX_SIZE+1];

	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(-1);
	}

	remote_server.sin_family=AF_INET;
	remote_server.sin_port=htons(atoi(tracker_info[target_seq].second.c_str()));
	remote_server.sin_addr.s_addr=inet_addr(tracker_info[target_seq].first.c_str());
	bzero(&remote_server.sin_zero,8);

	if((connect(sock,(struct sockaddr*)&remote_server,sizeof(struct sockaddr_in)))==-1){
		perror("connect");
		exit(-1);
	}

	string initial="SyncAll";
	send(sock,initial.c_str(),initial.size(),0);
}

void SyncAllHandler(int mysequence_i,vector<pair<string,string>> tracker_info){
	string path=".all_files";
	struct dirent *entry=NULL;
	DIR *dp=NULL;

	dp=opendir(path.c_str());

	if(dp!=NULL){

		while((entry=readdir(dp))){
			// cout<<entry->d_name<<endl;
			string filename(entry->d_name);

			cout<<"filename is "<<filename<<endl;

			if(filename=="." || filename=="..")
				continue;

			filename=path+"/"+filename;

			Sync(filename,mysequence_i,tracker_info);
		}


	}
	else{
		return;
	}

	closedir(dp);
	return;
}

void SyncRecv(int new_cli,string filename){
	cout<<"Recv"<<endl;
	int len=MAX_SIZE;
	char data[MAX_SIZE];
	// string line;
	ofstream outfile(filename,ios::out);
	cout<<filename<<endl;

	while(true){
		send(new_cli,"1",1,0);
		len=recv(new_cli,data,MAX_SIZE,0);
		data[len]='\0';

		string line(data);
		// cout<<line<<endl;
		if(line=="Sync Over")
			break;

		outfile<<line;
	}
	// cout<<"over"<<endl;
	outfile.close();
}