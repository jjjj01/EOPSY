//Jan Jakubiak, 289834
//EOPSY Laboratory 6
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Function displaying the correct syntax of the command copy
void help()
{
    printf("The correct syntax of the commend copy is as follows:\n");
    printf("copy [-m] <file_name> <new_file_name>\n");
    printf("copy [-h]\n");
    printf("Argument -m can be detected in any place, but names of the source file and the new file have to be in the correct order.\n");
}

void copy_read_write(int fd_from, int fd_to);
void copy_mmap(int fd_from, int fd_to);

int main(int argc, char *argv[])
{
    //Integer used as a flag to indicate if option -m was chosen
    int m=0;
    //Two strings of length 20 to store the names of the files
    char sourcename[20], targetname[20];
    //We run through arguments using getopt() and check for arguments -m and -h, acting appropriately.
    int option;
    while((option = getopt(argc, argv, ":mh"))!=-1)
    {
        switch(option)
        {
            case 'm':
                m=1;
                break;
            case 'h':
                help();
                return 0;
            case '?':
                help();
                return 0;
        }
    }

    //We check if the number of arguments is correct - we need either 2 or 3 arguments (depending on whether or not option -m was chosen)
    if(argc<3)
    {
        printf("Run the program again with correct number of arguments\n");
        help();
        return 0;
    }
    if(argc>4)
    {
        printf("Too many arguments. Run the function again with the correct syntax\n");
        help();
        return 0;
    }

    //If option -m was not chosen, we need to have 2 arguments - both filenames. We check for the number to be equal to 3, because "./copy" is read as an argument too.
    if(m==0)
    {
        if(argc==3)
        {
            //When we know that we have the correct name of arguments, we copy the name of files we'll be working with from the argument line
            strcpy(sourcename, argv[1]);
            strcpy(targetname, argv[2]);
            int source, target;
            //We check if the source file exists, to prevent errors
            if(access(sourcename,F_OK)==0)
            {
                //We open the source file with read-only permissions
                source=open(sourcename, O_RDONLY);
                //And either open the target file with write-only permissions, or create it with appropriate permissions
                target=open(targetname, O_WRONLY|O_CREAT, 0666);
                //We call the function responsible for copying the file contents, and close both files and return 0 afterwards 
                copy_read_write(source, target);
                close(source);
                close(target);
                return 0;
            }
            else
            {
                printf("That file does not exist.\n");
                help();
                return 0;
            }            
        }else{
            help();
            return 0;
        }
    }

    //If option -m was chosen, we need to have 3 arguments - -m and both filenames. As previously, we check for the number to be equal to 4 instead of 3 because the command name is read as an argument too.
    if(m==1)
    {
        if(argc==4)
        {
            int source, target;
            int name=0;
            //To enable using the option -m in any place on the command line, we use a simple for loop and traverse through all arguments from the command line. The first argument not equal to -m that we find is our sourcename, and the second our targetname.
            for(int i=1;i<argc;i++)
            {
                if(strcmp(argv[i], "-m")!=0)
                {
                    if(name==0)
                    {
                        strcpy(sourcename,argv[i]);
                        name=1;
                    }
                    if(name==1)
                    {
                        strcpy(targetname,argv[i]);
                        name==2;
                    }
                }
            }

            //We check is the source file exists
            if(access(sourcename,F_OK)==0)
            {
                //And we open the files and call the appropriate function, analogically to the situation without -m called.
                source=open(sourcename,O_RDONLY);
                target=open(targetname, O_RDWR|O_CREAT, 0666);
                copy_mmap(source, target);
                close(source);
                close(target);
                return 0;
            }
        }else{
            help();
            return 0;
        }
    }
}


void copy_read_write(int fd_from, int fd_to)
{
    //We create the stat structure and use it to get the size of the source file
    struct stat source_stat;
    fstat(fd_from, &source_stat);
    size_t size = source_stat.st_size;

    //we create the buffer of the size of the source file
    char buff[size];
    int bread, bwritten;
    //We traverse through the source file byte by byte, read each byte and immediately write it into the target file. Function read() returns 0 at the end of the file or -1 in case of an error, allowing us to use it as the termination condition for the loop
    while((bread=read(fd_from, buff, size))>0)
    {
        bwritten=write(fd_to, buff, bread);
    }
}



void copy_mmap(int fd_from, int fd_to)
{
    //We create the stat structure and use it to get the size of the source file
    struct stat source_stat;
    fstat(fd_from, &source_stat);
    size_t size = source_stat.st_size;
    
    //We create buffers for the whole files and map the addresses of the files into the buffers
    char* source_buff;
    source_buff=mmap(NULL, size,PROT_READ,MAP_SHARED,fd_from,0);
    char* target_buff;
    target_buff=mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_to, 0);

    //We change the size of the target file to fit the file we're copying there
    ftruncate(fd_to, size);

    //We copy the mapping from the soure-file buffer to the target-file buffer, effectively copying the file contents themselves, and we unmap the files to free the memory
    memcpy(target_buff, source_buff, size);
    munmap(source_buff, size);
    munmap(target_buff, size);
}
