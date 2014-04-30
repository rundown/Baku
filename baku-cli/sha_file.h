#include <openssl/sha.h>
#include <stdio.h>

using namespace std;

#define SHA_BUFFER_SIZE 8192

bool sha1_file(std::string name, unsigned char *out)
{
	FILE *f;
	unsigned char buf[SHA_BUFFER_SIZE];
	SHA256_CTX context;

	if (fopen_s(&f, name.c_str(), "rb") == 0) {
		SHA256_Init(&context);
		size_t len;
		while (len = fread(buf, 1, sizeof buf, f)){
			SHA256_Update(&context, buf, len);
		}
		fclose(f);
		SHA256_Final(out, &context);
		return true;
	}
	return false;
}
