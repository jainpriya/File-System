/* **********************ADVANCED OPERATING SYSTEM**********************

OBJECTIVE: To read a given directory and print the following information for all files in the directory
			-owner
			-filetype
			-access permissions
			-access time
		   It should be able to handle symbolic links and loops formed if any;

INPUT:     Path of the directory to be traversed

OUTPUT:    All the files and sub-directories information present in the directory given in input;

Approach:  Given a directory it will return a dirent structure using opendir() command.the readdir() command will return directory entries one by one,which will be proccessed one-byone
		   depending upon the file type i.e,directory or file or symbolic links via the defined function ProcessingEntity().Any loop formed via symbolic links is handled by maintaining a 
		   vecor of i-nodes.Whenever a file/directory is processed it's i-node is saved in the vectors and compared whenever a symbolic link is traversed so as to catch any loop in the directory.
*/

using namespace std;  
#include<iostream>   
#include<dirent.h>	//access directory function i.e opendir(),readdir();
#include<string.h>  //manipulates string
#include<vector>	//implement vector
#include<cstring>	
#include<sys/types.h>  //using stat and lstat
#include<unistd.h>	//getcwd
#include<sys/stat.h>	//using stat and lstat
#include<pwd.h>			//get owner information
#include<time.h>		//get access time
#include<algorithm>

vector<long int> InodeList;
vector<long int> :: iterator i; 
void ProcessDirectory(char *); //process directories
void ProcessEntity(struct dirent *); //process any entity that is present in directory
int CheckEntityType(struct dirent *);	//to check whether the entity is file/directory/symbolic links
string getpermission(char * filename);	//get access permissions
bool CheckProcessingInode(long int);    //check if i-node is present 



int main(int argc, char *argv[])
{
	if(argv[1]!= NULL)
	{
		cout<<"\tfilename\t\tfile_type\towner\tpermission\taccess_time\n\n";
		ProcessDirectory(argv[1]);
	}
	return 0;
}

/* Function definition to process the directory*/
void ProcessDirectory(char *ProcessingDirectory)
{
	DIR *dp;		//pointer to directory type
	struct dirent *entity;  //pointer to directory structure
	chdir(ProcessingDirectory);
	if((dp = opendir(ProcessingDirectory)) == NULL)    //if directory doesn't exist;
	{
		cerr<<"Cannot open the directory:"<< ProcessingDirectory<< endl;
		return ;
	}

	while((entity = readdir(dp)) != NULL)  		//read directory entries one-by-one
	{
		ProcessEntity(entity);                  //call ProcessEntity function to process the directory entry
	}
	closedir(dp);
}

void ProcessEntity(struct dirent *ProcessingEntity)		//process each entity of directory whether subdirectory/file/symbolic links
{
	char path[256];
	struct stat statbuff , s_statbuff; 		//buffer for stat and lstat
	ssize_t r; 								// size of buffer of linkname;
	char *linkname;							//for storing the file which is pointed by symbolic link
	lstat(ProcessingEntity->d_name, &statbuff);
	stat(ProcessingEntity->d_name, &s_statbuff);
	struct passwd *pw = getpwuid(statbuff.st_uid);
	int FileType = CheckEntityType( ProcessingEntity);   //check whether entity is file/directory/symbolic link
	bool IsAlreadyVisitedNode = CheckProcessingInode(statbuff.st_ino); //check whether the entity is already processed
	if(IsAlreadyVisitedNode==false)                //only if the entity is not processed then only it will processed and printed
	{
		InodeList.push_back(static_cast<long int>(statbuff.st_ino));

		if( FileType==0)			//file type
		{
			cout<<"\t"<<ProcessingEntity->d_name <<"\t\t"<<"REGULAR"<<"\t"<<pw->pw_name<<"\t"<<getpermission(ProcessingEntity->d_name)<<"\t"<<ctime(&statbuff.st_atime)<<"\n\n";
		}

		if( FileType==1)			//directory type
		{
			if( string(ProcessingEntity->d_name).compare(".") ==0 || string(ProcessingEntity->d_name).compare("..") == 0)  //no current and parent directory should be printed
				return;
			cout<<"\t\t"<<ProcessingEntity->d_name <<"\tDirectory"<<pw->pw_name<<"\t"<<getpermission(ProcessingEntity->d_name)<<"\t"<<ctime(&statbuff.st_atime)<<"\n\n";


			//Getting path of the Directory for next entity to be printed for sub directory
			getcwd(path, sizeof(path));
			char *temp = new char[std::strlen(path)+strlen("/") + strlen(ProcessingEntity->d_name)];
			std::strcpy(temp, path);
			std::strcat(temp, "/");
			std::strcat(temp , ProcessingEntity->d_name);


			ProcessDirectory(temp);
			delete[] temp;
		}

		if( FileType == 2)   //symbolic links
		{	
			struct stat l_link_buffer; //buffer for lstat of linkname
			linkname= new char(s_statbuff.st_size+1); //size of file which is pointed by symbolic link
			if(linkname==NULL)
			{
				cout<<"insufficient memory";
				exit(0);
			}
			r=readlink(ProcessingEntity->d_name,linkname,s_statbuff.st_size+1); //read the file name which is pointed by the symbolic link
				if(r<0)   //for special case files it will return 0;
				{
					exit(0);
				}
				if(r>s_statbuff.st_size+1)
				{
					cout<<"Symbolic link size increased in size\n";
					exit(0);
				}
				linkname[r+1]='\0';   //add null character for path
				cout<<ProcessingEntity->d_name<<"  points to"<<"\t"<<linkname<<"\n\n";
				
				if(CheckProcessingInode(static_cast<long int>(s_statbuff.st_ino))== false)  //if the file is already processed it will form loop
				{
					
					//to check if the linkname is a file/directory

					lstat(linkname,&l_link_buffer);
					if(l_link_buffer.st_mode & S_IFMT == S_IFDIR)
					{
						//Getting path of the Directory
						getcwd(path, sizeof(path));
						char *temp = new char[std::strlen(path)+strlen("/") + strlen(linkname)];
						std::strcpy(temp, path);
						std::strcat(temp, "/");
						std::strcat(temp ,linkname);
						ProcessDirectory(temp);
						delete[] temp;
					}
					else if(l_link_buffer.st_mode & S_IFMT == S_IFREG)		
					{//it is a file print as it is
						InodeList.push_back(static_cast<long int>(s_statbuff.st_ino));

						struct passwd *link_pw = getpwuid(s_statbuff.st_uid);
						cout<<"\t"<<linkname <<"\t\t"<<"REGULAR"<<"\t"<<link_pw->pw_name<<"\t"<<getpermission(linkname)<<"\t"<<ctime(&s_statbuff.st_atime)<<"\n\n";
					}
				}

				else
				{
					cout<<"loop exists at :\n"<<"\t"<<linkname<<endl;
					return;
				}
		}		
	}
	else
	{

		cout<<"loop exists at:"<<"\t"<<ProcessingEntity->d_name<<endl;
		return;
	}

	return;
}

int CheckEntityType(struct dirent *ProcessingEntity)
{
    switch (ProcessingEntity->d_type) 
    {
        case DT_BLK :   
				break;
		 case DT_CHR :
				break;
		 case DT_DIR :   //directory 
				return 1;
				break;
         case DT_FIFO :  
				break;
		 case DT_LNK :   //symbolic link
				return 2;
				break;
		 case DT_REG :   //files
		 		return 0;
				break;
		 case DT_SOCK :   
				break;
		
		 case DT_UNKNOWN: 
				break;
		default :
				break;
    }

    
}

string getpermission(char *filename)
{
        struct stat statbuff;
        lstat(filename,&statbuff);
        string per;
        mode_t val = statbuff.st_mode & ~S_IFMT;
        (val & S_IRUSR) ? per.append("r") : per.append("-");
        (val & S_IWUSR) ? per.append("w") : per.append("-");    
        (val & S_IXUSR) ? per.append("x") : per.append("-");
        (val & S_IRGRP) ? per.append("r") : per.append("-");
        (val & S_IWGRP) ? per.append("w") : per.append("-");
        (val & S_IXGRP) ? per.append("x") : per.append("-");
        (val & S_IROTH) ? per.append("r") : per.append("-");
        (val & S_IWOTH) ? per.append("w") : per.append("-");
        (val & S_IXOTH) ? per.append("x") : per.append("-");

        return per;
 }

 bool CheckProcessingInode(long int ProcessingNode)   //check if i-node is presnt in the vector i.e, already processed.
 {
 	sort(InodeList.begin() , InodeList.end());
 	return binary_search(InodeList.begin() , InodeList.end() , ProcessingNode);
 }
