#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	struct servent *s_ent;
	if (argc < 2) return -1;
	switch (argv[1][0]) {
	case '0':
		s_ent = getservbyname("https", NULL);
		break;
	case '1':
		s_ent = getservbyport(htons(22), NULL);
		break;
	default:
		return -2;
	}
	if (!s_ent)
		return 1;
	if (!__builtin_cheri_tag_get(s_ent))
		return 2;
	if (!(s_ent->s_name && s_ent->s_port && s_ent->s_proto))
		return 3;
	if (__builtin_cheri_length_get(s_ent) != sizeof(struct servent))
		return 4;
}
