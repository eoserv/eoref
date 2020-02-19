#include <cassert>
#include <cstdio>

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::fputs("usage: bin2c infile outfile identifier", stderr);
		return 1;
	}

	const char* infile = argv[1];
	const char* outfile = argv[2];
	const char* identifier = argv[3];

	std::FILE* in = nullptr;
	std::FILE* out = nullptr;

	long length;
	in = std::fopen(infile, "rb");

	if (!in)
	{
		std::perror("Could not open input file");
		goto fail;
	}

	out = std::fopen(outfile, "wt");

	if (!out)
	{
		std::perror("Could not open output file");
		goto fail;
	}

	std::fseek(in, 0, SEEK_END);
	length = std::ftell(in);
	std::fseek(in, 0, SEEK_SET);

	if (length <= 0)
	{
		std::perror("Zero-size input file");
		goto fail;
	}

	{
		int i = 0;
		int c;

		std::fprintf(out, "#include <cstdlib>\n\n");
		std::fprintf(out, "unsigned char %s_bytes[] = {\n", identifier);

		while ((c = std::fgetc(in)) != EOF)
		{
			if (i == 0)
				std::fputc('\t', out);

			assert(c >= 0x00 && c <= 0xFF);
			std::fprintf(out, "0x%02x, ", c);

			if (i++ == 8)
			{
				std::fputc('\n', out);
				i = 0;
			}
		}

		if (i != 0)
			std::fputc('\n', out);

		std::fprintf(out, "};\n");
		std::fprintf(out, "\nstd::size_t %s_size = sizeof %s_bytes;\n\n", identifier, identifier);

		if (std::ferror(in))
		{
			std::perror("Input read failure");
			goto fail;
		}
	}

	return 0;

fail:
	if (out) std::fclose(out);
	if (in) std::fclose(in);

	return 1;
}
