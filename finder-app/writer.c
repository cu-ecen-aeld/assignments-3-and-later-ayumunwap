#include <stdio.h>
#include <syslog.h>
// the first argument is a full path to a file (including filename) on the filesystem
// the second argument is a text string which will be written within this file
int main(int argc, char *argv[]) {
    //Setup syslog logging using the LOG_USER facility.
    openlog(NULL, 0, LOG_USER);
    if (argc != 3) {
        syslog(LOG_ERR, "The number of provided arguments is wrong");
        return 1;
    }
    
    FILE *file = fopen(argv[1], "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error opening file");
        return 1;
    }
    
    char *text = argv[2];
    fprintf(file, "%s\n", text);
    fclose(file);
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);    
    return 0;
}
