#include <string.h>
//#include <locale.h> see todo below

int main(int argc, char **argv) {
    // TODO set local is not ported yet. It shouldn't change anything in the
    // current implementation, but might be an issue in the future if strcoll
    // is modified in the future : the test env might differ between people
    //setlocale(LC_ALL, "C.UTF-8");

    char* empty_string = "";
    char* low_string = "big";
    char* high_string =   "bigger";

    // The input parameters are const so there is no corruption of
    // capabilities possible. We only test to see if it runs.
    if ( strcoll(empty_string,low_string) >= 0) return 1;
    if ( strcoll(high_string,low_string) <= 0) return 2;
    if ( strcoll(low_string,low_string) != 0) return 3;

    return 0;
}
