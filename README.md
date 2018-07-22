# File-System
Given path of a directory, read the directory and print information for all files and sub-directories. The information includes owner name, file type, access permissions and access time. It also detects any symbolic link (if exists) in the given directory.

Approach: Given a directory it will return a dirent structure using opendir() command.the readdir() command will return directory entries one by one,which will be proccessed one-by-one depending upon the file type i.e, directory/file/ symbolic links via the defined function ProcessingEntity().Any loop formed via symbolic links is handled by maintaining a vector of i-nodes. Whenever a file/directory is processed it's i-node is saved in the vectors and compared whenever a symbolic link is traversed so as to catch any loop in the directory.


