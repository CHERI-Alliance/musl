#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int check_result(size_t len, size_t expected_len, wchar_t* wcp, mbstate_t* stp)
{
    unsigned return_value = 0;
    if (len < 0) {
        printf("error in mbrtowc, negative return");
        return_value |= 1;
    }
    /* TODO once setlocale is ported, reactivate this code. Hopefully mbrtowc's loop works well and
    read the correct amount of character.
    if (len != expected_len) {
        printf("mbrtowc did not read the correct number of byte. Expected %zu but got %zu\n",expected_len,len);
        return_value |= 2;
    }
    */
    if (!__builtin_cheri_tag_get(wcp) || !__builtin_cheri_tag_get(stp)) {
        printf("mbrtowc cleared the capability tag of its IO parameters");
        return_value |= 4;
    }
    if (*(unsigned *) stp != 0) {
        printf("mbrtowc outputed a non clean mbstate");
        return_value |= 8;
    }

    if (return_value) {
        printf("chars %hhx %hhx %hhx %hhx (assuming little endian)\n",((char*) wcp)[3],((char*) wcp)[2],((char*) wcp)[1],((char*) wcp)[0]);
    }
    return return_value;
}

int main(void) {
    //setlocale(LC_ALL, "en_GB.utf8"); //TODO once setlocale is ported, reactivate this code.

    wchar_t wc = 0;
	mbstate_t st = (mbstate_t){0};
    wchar_t* wcp = &wc;
	mbstate_t* stp = &st;
    const char* src = "z\u00df\u6c34\U0001d10b"; // or u8"zß水𝄋"
                      // or "\x7a\xc3\x9f\xe6\xb0\xb4\xf0\x9d\x84\x8b";
    unsigned int remaining_char = 11;
    int return_value = 0;
    size_t len;

    len = mbrtowc(wcp, src, remaining_char, stp);
    if ((return_value = check_result(len,1,wcp,stp))) return return_value;
    src += len, remaining_char -= len;

    len = mbrtowc(wcp, src+1, remaining_char-1, stp);
    if ((return_value = check_result(len,2,wcp,stp))) return return_value;
    src += len, remaining_char -= len;

    len = mbrtowc(wcp, src, remaining_char, stp);
    if ((return_value = check_result(len,3,wcp,stp))) return return_value;
    src += len, remaining_char -= len;

    len = mbrtowc(wcp, src, remaining_char, stp);
    if ((return_value = check_result(len,4,wcp,stp))) return return_value;
    src += len, remaining_char -= len;

    return 0;
}
