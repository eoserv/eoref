
#include "ast.hpp"
#include "eo_protocol_parser.hpp"
#include "lex.hpp"

#include "printer.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <utility>

static constexpr std::array headers{
	// for eo_int, etc...
	"data/eo_types.hpp"
};

static constexpr std::array struct_headers = {
	// for EO_Stream_Reader / EO_Stream_Builder
	"data/eo_stream.hpp"
};

static constexpr std::array packet_headers = {
	// for PacketFamily, PacketAction, Client_Packet, Server_Packet
	"packet/packet_base.hpp"
};

template <class... Args>
void write_includes(std::ostream& f, Args&&... args)
{
	for (auto c : {args...})
		for (auto s : c)
			f << "#include \"" << s << "\"\n";
}


static const char* autogen_comment = \
	"// ====================================\n" \
	"// === WARNING: AUTO-GENERATED CODE ===\n" \
	"// ====================================\n";

static void write_enums(std::ostream& f, const Printer& p)
{
	f << "#ifndef EO_PROTOCOL_ENUMS_HPP\n";
	f << "#define EO_PROTOCOL_ENUMS_HPP\n\n";
	f << autogen_comment << "\n";
	write_includes(f, headers);
	f << "namespace eo_protocol\n{\n\n\n";
	p.print_enums(f);
	f << "\n}\n";
	f << "\n#endif // EO_PROTOCOL_ENUMS_HPP\n";
	f << std::endl;
}

static void write_structs(std::ostream& f, const Printer& p)
{
	f << "#ifndef EO_PROTOCOL_STRUCTS_HPP\n";
	f << "#define EO_PROTOCOL_STRUCTS_HPP\n\n";
	f << autogen_comment << "\n";
	write_includes(f, headers);
	f << "#include \"enums.hpp\"\n\n";
	f << "#include <array>\n";
	f << "#include <numeric>\n";
	f << "#include <string>\n";
	f << "#include <vector>\n\n";
	f << "class EO_Stream_Reader;\nclass EO_Stream_Builder;\n\n\n";
	f << "namespace eo_protocol\n{\n\n\n";
	p.print_structs(f);
	f << "\n}\n";
	f << "\n#endif // EO_PROTOCOL_STRUCTS_HPP\n";
	f << std::endl;
}

static void write_struct_impls(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	f << "#include \"structs.hpp\"\n\n";
	write_includes(f, headers, struct_headers);
	f << "namespace eo_protocol\n{\n\n\n";
	p.print_struct_impls(f);
	f << "\n}\n";
	f << std::endl;
}

static void write_pub_enums(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	f << "#ifndef EO_PROTOCOL_PUB_ENUMS_HPP\n";
	f << "#define EO_PROTOCOL_PUB_ENUMS_HPP\n\n";
	write_includes(f, headers);
	f << "namespace eo_protocol\n{\n\n\n";
	p.print_enums(f);
	f << "\n}\n";
	f << "\n#endif // EO_PROTOCOL_PUB_ENUMS_HPP\n";
	f << std::endl;
}

static void write_pub_structs(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	f << "#ifndef EO_PROTOCOL_PUB_STRUCTS_HPP\n";
	f << "#define EO_PROTOCOL_PUB_STRUCTS_HPP\n\n";
	write_includes(f, headers);
	f << "#include \"pub_enums.hpp\"\n\n";
	f << "#include <array>\n";
	f << "#include <numeric>\n";
	f << "#include <string>\n";
	f << "#include <vector>\n\n";
	f << "class EO_Stream_Reader;\nclass EO_Stream_Builder;\n\n\n";
	f << "namespace eo_protocol\n{\n\n\n";
	p.print_structs(f);
	f << "\n}\n";
	f << "\n#endif // EO_PROTOCOL_PUB_STRUCTS_HPP\n";
	f << std::endl;
}

static void write_pub_struct_impls(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	f << "#include \"pub_structs.hpp\"\n\n";
	write_includes(f, headers, struct_headers);
	f << "namespace eo_protocol\n{\n\n\n";
	p.print_struct_impls(f);
	f << "\n}\n";
	f << std::endl;
}

static void write_client_packets(std::ostream& f, const Printer& p)
{
	f << "#ifndef EO_PROTOCOL_CLIENT_HPP\n";
	f << "#define EO_PROTOCOL_CLIENT_HPP\n\n";
	f << autogen_comment << "\n";
	write_includes(f, headers, packet_headers);
	f << "#include <array>\n";
	f << "#include <numeric>\n";
	f << "#include <string>\n";
	f << "#include <vector>\n\n";
	f << "#include \"structs.hpp\"\n\n";
	f << "class EO_Stream_Reader;\nclass EO_Stream_Builder;\n\n\n";
	f << "namespace eo_protocol\n{\n\n\n";
	f << "namespace client\n{\n\n\n";
	p.print_client_packet_defs(f);
	f << "\n}\n}\n";
	f << "\n#endif // EO_PROTOCOL_CLIENT_HPP\n";
	f << std::endl;
}

static void write_client_packet_impls(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	f << "#include \"client.hpp\"\n\n";
	write_includes(f, headers, struct_headers, packet_headers);
	f << "namespace eo_protocol\n{\n";
	f << "namespace client\n{\n\n\n";
	p.print_client_packet_impls(f);
	f << "\n}\n}\n";
	f << std::endl;
}

static void write_client_packet_tpp(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	p.print_client_packet_cases(f);
	f << std::endl;
}

static void write_server_packets(std::ostream& f, const Printer& p)
{
	f << "#ifndef EO_PROTOCOL_SERVER_HPP\n";
	f << "#define EO_PROTOCOL_SERVER_HPP\n\n";
	f << autogen_comment << "\n";
	write_includes(f, headers, packet_headers);
	f << "#include <array>\n";
	f << "#include <string>\n";
	f << "#include <vector>\n\n";
	f << "#include \"structs.hpp\"\n\n";
	f << "class EO_Stream_Reader;\nclass EO_Stream_Builder;\n\n\n";
	f << "namespace eo_protocol\n{\n\n\n";
	f << "namespace server\n{\n\n\n";
	p.print_server_packet_defs(f);
	f << "\n}\n}\n";
	f << "\n#endif // EO_PROTOCOL_SERVER_HPP\n";
	f << std::endl;
}

static void write_server_packet_impls(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	f << "#include \"server.hpp\"\n\n";
	write_includes(f, headers, struct_headers, packet_headers);
	f << "namespace eo_protocol\n{\n";
	f << "namespace server\n{\n\n\n";
	p.print_server_packet_impls(f);
	f << "\n}\n}\n";
	f << std::endl;
}

static void write_server_packet_tpp(std::ostream& f, const Printer& p)
{
	f << autogen_comment << "\n";
	p.print_server_packet_cases(f);
	f << std::endl;
}

int main(int argc, char** argv)
{
	enum
	{
		mode_none,
		mode_net,
		mode_pub
	} mode = mode_none;

	const char* input_filename = nullptr;

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];

		if (arg[0] == '-')
		{
			if (std::strcmp(arg, "-net") == 0)
			{
				mode = mode_net;
			}
			else if (std::strcmp(arg, "-pub") == 0)
			{
				mode = mode_pub;
			}
			else
			{
				std::cerr << "Unknown option: " << arg << std::endl;
				return 1;
			}
		}
		else if (arg[0] != '\0')
		{
			if (!input_filename)
			{
				input_filename = arg;
			}
			else
			{
				std::cerr << "Multiple input files specified." << std::endl;
				return 1;
			}
		}
	}

	if (!input_filename)
	{
		std::cerr << "No input file specified" << std::endl;
		return 1;
	}

	if (!mode)
	{
		std::cerr << "No output mode specified" << std::endl;
		return 1;
	}

	std::ifstream ss(input_filename);

	if (!ss)
	{
		std::cerr << "Could not open input file " << input_filename << std::endl;
		return 1;
	}

	ProtocolFile::ptr ast;
	std::deque<Token> tokens;

	try
	{
		Lexer l(ss);
		l.Lex(std::back_inserter(tokens));

		EO_Protocol_Parser p(tokens.cbegin(), tokens.cend());
		p.ParseProtocolFile(ast);
	}
	catch (Lexer_Error& e)
	{
		std::cerr << "parsing failed: (line " << e.line() << ", col " << e.col() << ")\n\t"
		          << e.what() << std::endl;
		return 1;
	}
	catch (Syntax_Error& e)
	{
		std::cerr << "parsing failed: (line " << e.line() << ")\n\t"
		          << e.what() << std::endl;
		return 1;
	}

	Printer p(*ast);

	using FileListEntry = std::pair<const char*, void (*)(std::ostream&, const Printer&)>;

	FileListEntry net_files[] = {
		{ "enums.hpp", write_enums },
		{ "structs.hpp", write_structs },
		{ "structs.cpp", write_struct_impls },
		{ "client.hpp", write_client_packets },
		{ "client.cpp", write_client_packet_impls },
		{ "client_packets.tpp", write_client_packet_tpp },
		{ "server.hpp", write_server_packets },
		{ "server.cpp", write_server_packet_impls },
		{ "server_packets.tpp", write_server_packet_tpp }
	};

	FileListEntry pub_files[] = {
		{ "pub_enums.hpp", write_pub_enums },
		{ "pub_structs.hpp", write_pub_structs },
		{ "pub_structs.cpp", write_pub_struct_impls }
	};

	auto gen_files = [&](auto&& file_list)
	{
		for (auto&& [filename, fn] : file_list)
		{
			std::ofstream f(filename);

			if (!f)
			{
				std::cerr << "Could not open " << filename << " for writing" << std::endl;
				std::exit(1);
			}

			fn(f, p);
		}
	};

	if (mode == mode_net)
	{
		gen_files(net_files);
	}
	else if (mode == mode_pub)
	{
		gen_files(pub_files);
	}
}
