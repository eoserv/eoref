#include "printer.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <vector>
#include <utility>

static std::string map_type(const std::string& s)
{
	     if (s == "byte")          return "eo_byte";
	else if (s == "char")          return "eo_char";
	else if (s == "short")         return "eo_short";
	else if (s == "three")         return "eo_three";
	else if (s == "int")           return "eo_int";
	else if (s == "string")        return "std::string";
	else if (s == "raw_string")    return "std::string";
	else if (s == "prefix_string") return "std::string";
	else                           return s;
}

template <class K, class V>
std::vector<std::pair<K, V>>
make_sorted_by_key(const std::unordered_map<K, V>& m)
{
	std::vector<std::pair<K, V>> result;
	result.reserve(m.size());

	for (auto& p : m)
		result.push_back(p);

	std::sort(result.begin(), result.end(),
		[](const std::pair<K, V>& a, const std::pair<K, V>& b)
		{
			return a.first < b.first;
		}
	);

	return result;
}

template <class K, class V>
std::vector<std::pair<K, V>>
make_sorted_by_value(const std::unordered_map<K, V>& m)
{
	std::vector<std::pair<K, V>> result;
	result.reserve(m.size());

	for (auto& p : m)
		result.push_back(p);

	std::sort(result.begin(), result.end(),
		[](const std::pair<K, V>& a, const std::pair<K, V>& b)
		{
			return a.second < b.second;
		}
	);

	return result;
}

static std::string make_tabs(int depth)
{
	return std::string(depth, '\t');
}

static std::string prefix_lines(std::string s, const std::string& pre)
{
	std::size_t pos = -1;

	if (!s.empty() && s.back() == '\n')
		s.pop_back();

	while ((pos = s.find('\n', pos + 1)) != std::string::npos)
	{
		s.replace(pos, 1, "\n" + pre);
	}

	return pre + s + '\n';
}

struct make_byte_size_visitor
{
	const Printer& printer;
	const DataBlockEntries& dbe;
	int& constant;
	std::string& adds;
	std::string prefix;

	void operator()(const std::shared_ptr<DataField>& data_field)
	{
		if (data_field->type == "string" || data_field->type == "raw_string"
		 || data_field->type == "prefix_string")
		{
			bool has_break = (data_field->type == "string" || data_field->type == "prefix_string");

			if (data_field->static_size || data_field->dynamic_size || data_field->implicit_size)
			{
				adds += "\n\t      + std::accumulate(" + prefix + data_field->name.value() + ".begin(), "
				      + prefix + data_field->name.value() + ".end(), std::size_t(0), [](std::size_t i, const auto& x) "
				        "{ return i + x.size()" + (has_break ? " + 1" : "") + "; })";
			}
			else
			{
				adds += "\n\t      + " + prefix + data_field->name.value() + ".size()";

				if (has_break)
					constant += 1;
			}
		}
		else
		{
			auto type_size = printer.size_of(data_field->type);

			if (data_field->static_size || data_field->dynamic_size || data_field->implicit_size)
			{
				adds += "\n\t      + (" + std::to_string(type_size)
					  + " * " + prefix + data_field->name.value() + ".size())";
			}
			else
				constant += data_field->static_size.value_or(1) * printer.size_of(data_field->type);
		}
	}

	void operator()(const std::shared_ptr<StructField>& struct_field)
	{
		if (struct_field->static_size || struct_field->dynamic_size || struct_field->implicit_size)
		{
			adds += "\n\t      + std::accumulate(" + prefix + struct_field->name.value() + ".begin(), "
			      + prefix + struct_field->name.value() + ".end(), std::size_t(0), [](std::size_t i, const auto& x) "
			        "{ return i + x.byte_size(); })";
		}
		else
			adds += "\n\t      + " + prefix + struct_field->name.value() + ".byte_size()";
	}

	void operator()(const std::shared_ptr<UnionBlock>& union_block)
	{
		int max_constant = 0;

		{
			auto enum_type = printer.get_enum_type(dbe, union_block->switch_field);

			for (auto& case_it : union_block->cases)
			{
				auto& case_name = case_it.first;
				auto& case_data = case_it.second;

				int inner_constant = 0;
				std::string inner_adds;

				make_byte_size_visitor inner_size{printer, dbe, inner_constant,
					                              inner_adds, "u." + case_data->name + "."};

				for (auto& entry : case_data->dbe.entries)
				{
					visit(inner_size, entry);
				}

				if (!inner_adds.empty())
				{
					inner_adds = inner_adds.substr(7);

					adds += "\n\t     + ("
						  + union_block->switch_field + " == "
					      + enum_type + "::" + case_name + " ? "
						  + "(" + std::to_string(inner_constant) + inner_adds + ")"
						  + " : 0)";
				}
				else
				{
					if (inner_constant > max_constant)
						max_constant = inner_constant;
				}
			}
		}
	}
};

const EnumBlock& Printer::get_enum(const std::string& name) const
{
	auto it = m_proto.enums.find(name);

	if (it == m_proto.enums.end())
		throw std::runtime_error("Unknown enum: " + name);

	return *it->second;
}

std::string Printer::get_enum_type(const DataBlockEntries& dbe, const std::string& field_name) const
{
	for (auto& entry_it : dbe.entries)
	{
		if (entry_it.index() != 0)
			continue;

		auto& data_field = *std::get<std::shared_ptr<DataField>>(entry_it);

		if (data_field.name == field_name)
			return data_field.type;
	}

	throw std::runtime_error("Switching on non-existent field: " + field_name);
}

std::size_t Printer::size_of(const std::string& s) const
{
	     if (s == "byte")          return 1;
	else if (s == "char")          return 1;
	else if (s == "short")         return 2;
	else if (s == "three")         return 3;
	else if (s == "int")           return 4;
	else if (s == "string")        return 9999;
	else if (s == "raw_string")    return 9999;
	else if (s == "prefix_string") return 9999;
	else if (s == "break")         return 1;
	else
	{
		auto& enum_block = get_enum(s);
		return size_of(enum_block.type_class.value_or("int"));
	}
}

std::string Printer::add_fn(const std::string& s) const
{
	     if (s == "byte")          return "add_byte";
	else if (s == "char")          return "add_char";
	else if (s == "short")         return "add_short";
	else if (s == "three")         return "add_three";
	else if (s == "int")           return "add_int";
	else if (s == "string")        return "add_break_string";
	else if (s == "raw_string")    return "add_string";
	else if (s == "prefix_string") return "add_prefix_string";
	else if (s == "break")         return "add_byte(0xFF)";
	else
	{
		auto& enum_block = get_enum(s);
		return add_fn(enum_block.type_class.value_or("int"));
	}
}

std::string Printer::get_fn(const std::string& s) const
{
	     if (s == "byte")          return "get_byte";
	else if (s == "char")          return "get_char";
	else if (s == "short")         return "get_short";
	else if (s == "three")         return "get_three";
	else if (s == "int")           return "get_int";
	else if (s == "string")        return "get_break_string";
	else if (s == "raw_string")    return "get_end_string";
	else if (s == "prefix_string") return "get_prefix_string";
	else if (s == "break")         return "get_byte";
	else
	{
		auto& enum_block = get_enum(s);
		return get_fn(enum_block.type_class.value_or("int"));
	}
}

std::string Printer::enum_base_type(const std::string& s) const
{
	     if (s == "byte")          return s;
	else if (s == "char")          return s;
	else if (s == "short")         return s;
	else if (s == "three")         return s;
	else if (s == "int")           return s;
	else if (s == "string")        return s;
	else if (s == "raw_string")    return s;
	else if (s == "prefix_string") return s;
	else if (s == "break")         return s;
	else
	{
		auto& enum_block = get_enum(s);
		return enum_base_type(enum_block.type_class.value_or("int"));
	}
}

std::string Printer::make_byte_size_expression(DataBlockEntries& dbe) const
{
	int constant = 0;
	std::string adds;

	for (auto& entry : dbe.entries)
	{
		visit(make_byte_size_visitor{*this, dbe, constant, adds, {}}, entry);
	}

	return std::to_string(constant) + adds;
}

Printer::Printer(ProtocolFile& proto)
: m_proto(proto)
{ }

Printer::TypeType_t Printer::type_type(const std::string& name) const
{
	if (name == "break")
		return type_break;
	else
	{
		auto it = m_proto.enums.find(name);

		if (it == m_proto.enums.end())
			return type_builtin;
		else
			return type_enum;
	}
}

struct data_block_print_visitor
{
	const Printer& printer;
	std::ostream& os;
	const ProtocolFile& proto;
	const int depth;
	const std::string& tabs;

	void operator()(const std::shared_ptr<DataField>& data_field)
	{
		if (data_field->static_size)
		{
			std::string base_type = data_field->type;

			if (data_field->type_class)
				base_type = data_field->type_class.value();

			if (data_field->name)
			{
				os << tabs << "std::array<" << map_type(base_type) << ", "
				   << data_field->static_size.value() << "> "
				   << data_field->name.value() << ";\n";
			}
		}
		else if (data_field->dynamic_size || data_field->implicit_size)
		{
			std::string base_type = data_field->type;

			if (data_field->type_class)
				base_type = data_field->type_class.value();

			if (data_field->name)
			{
				os << tabs << "std::vector<" << map_type(base_type) << "> "
				   << data_field->name.value() << ";\n";
			}
		}
		else
		{
			std::string base_type = data_field->type;

			if (data_field->type_class)
				base_type = data_field->type_class.value();

			if (data_field->name)
			{
				os << tabs << map_type(base_type) << ' '
				   << data_field->name.value() << ";\n";
			}
		}
	}

	void operator()(const std::shared_ptr<StructField>& struct_field)
	{
		if (struct_field->static_size)
		{
			if (struct_field->name)
			{
				os << tabs << "std::array<" << struct_field->type << ", "
				   << struct_field->static_size.value() << "> "
				   << struct_field->name.value() << ";\n";
			}
		}
		else if (struct_field->dynamic_size || struct_field->implicit_size)
		{
			if (struct_field->name)
			{
				os << tabs << "std::vector<" << struct_field->type << "> "
				   << struct_field->name.value() << ";\n";
			}
		}
		else
		{
			if (struct_field->name)
			{
				os << tabs << struct_field->type << ' '
				   << struct_field->name.value() << ";\n";
			}
		}
	}

	void operator()(const std::shared_ptr<UnionBlock>& union_block)
	{
		os << '\n';

		os << tabs << "union u_t\n"
		   << tabs << "{\n";

		std::size_t i = 0;

		for (const auto& case_it : union_block->cases)
		{
			//auto& case_value = case_it.first;
			auto& case_data = *case_it.second;

			os << tabs << "\tstruct " << case_data.name << "_t\n"
			   << tabs << "\t{\n";

			printer.print_data_block(os, case_data.dbe, depth+2);

			os << tabs << "\t} " << case_data.name << ";\n";

			if (i++ != union_block->cases.size() - 1)
				os << '\n';
		}

		os << '\n';
		os << tabs << "\tu_t() { }\n";
		//os << tabs << "\tu_t(const u_t&) { } // Bad: breaks copying\n";
		//os << tabs << "\tu_t& operator=(const u_t&) { return *this; } // Bad: breaks copy-assigning\n";
		os << tabs << "\t~u_t() { }\n";
		os << tabs << "} u;\n";
	}
};

void Printer::print_data_block(std::ostream& os, const DataBlockEntries& dbe,
                               int depth) const
{
	auto tabs = make_tabs(depth);

	for (auto& entry : dbe.entries)
	{
		std::visit(data_block_print_visitor{*this, os, m_proto, depth, tabs}, entry);
	}

	if (!dbe.functions.empty())
		os << '\n';

	for (auto& entry : dbe.functions)
	{
		os << tabs << entry->signature << '\n'
		   << tabs << "{\n"
		   << prefix_lines(entry->body, tabs + '\t')
		   << tabs << "}\n";
	}
}

struct serialize_print_visitor
{
	const Printer& printer;
	const DataBlockEntries& dbe;
	std::ostream& os;
	const ProtocolFile& proto;
	const int depth;
	const std::string& tabs;
	const std::string& prefix;

	struct wrap_result_t
	{
		std::function<void()> print_prefix;
		std::function<void()> print_suffix;
		std::function<std::string(const std::string&)> wrap_id;
		std::string tabs;
	};

	wrap_result_t wrap_field(std::optional<int> static_size,
	                         std::optional<std::string> dynamic_size,
	                         bool implicit_size, std::optional<std::string> name)
	{
		wrap_result_t result;

		result.print_prefix = []() { };
		result.print_suffix = []() { };
		result.wrap_id = [](const std::string& s) { return s; };

		std::optional<std::string> loop_cond;
		std::string loop_idx;

		if (name)
		{
			if ((static_size || dynamic_size || implicit_size))
				loop_cond = "i < " + prefix + name.value() + ".size()";
		}
		else
		{
			if ((static_size || dynamic_size || implicit_size))
				throw std::runtime_error("Anonymous field should not have a size");
		}

		if (loop_cond)
		{
			result.tabs = make_tabs(depth + 1);

			result.print_prefix = [=]()
			{
				os << tabs << "for (std::size_t i = 0; "
				   << loop_cond.value()  << "; ++i)\n";
			};

			result.wrap_id = [=](const std::string& s)
			{
				return s + "[i]";
			};
		}
		else
		{
			result.tabs = tabs;
		}

		return result;
	}

	void operator()(const std::shared_ptr<DataField>& data_field)
	{
		auto wrap = wrap_field(data_field->static_size, data_field->dynamic_size,
		                       data_field->implicit_size, data_field->name);

		std::string base_type = data_field->type;

		if (data_field->type_class)
			base_type = data_field->type_class.value();

		base_type = printer.enum_base_type(base_type);

		std::string add = printer.add_fn(base_type);

		std::string cast_begin;
		std::string cast_end;

		if (printer.type_type(data_field->type) == Printer::type_enum)
		{
			cast_begin = "static_cast<" + map_type(base_type) + ">(";
			cast_end = ")";
		}

		wrap.print_prefix();

		if (data_field->name)
		{
			os << wrap.tabs << "builder." << add << "(" << cast_begin
			   << wrap.wrap_id(prefix + data_field->name.value()) << cast_end << ");\n";
		}
		else if (data_field->initializer)
		{
			os << wrap.tabs << "builder." << add << "(" << cast_begin
			   << data_field->initializer.value() << cast_end << ");\n";
		}
		else
		{
			os << wrap.tabs << "builder." << add << ";\n";
		}

		wrap.print_suffix();
	}

	void operator()(const std::shared_ptr<StructField>& struct_field)
	{
		auto wrap = wrap_field(struct_field->static_size, struct_field->dynamic_size,
		                       struct_field->implicit_size, struct_field->name);

		wrap.print_prefix();

		os << wrap.tabs << prefix << wrap.wrap_id(struct_field->name.value())
		   << ".serialize(builder);\n";

		wrap.print_suffix();
	}

	void operator()(const std::shared_ptr<UnionBlock>& union_block)
	{
		os << '\n';

		os << tabs << "switch (" << prefix << union_block->switch_field << ")\n"
		   << tabs << "{\n";

		auto enum_type = printer.get_enum_type(dbe, union_block->switch_field);

		std::size_t i = 0;

		for (const auto& case_it : union_block->cases)
		{
			auto& case_value = case_it.first;
			auto& case_data = *case_it.second;

			std::string new_prefix = prefix + "u." + case_data.name + ".";

			if (case_value == "default")
				os << tabs << "\tdefault:\n";
			else
				os << tabs << "\tcase " << enum_type << "::" << case_value << ":\n";

			printer.print_serialize_code(os, case_data.dbe, depth+2, new_prefix);

			os << tabs << "\tbreak;\n";

			if (i++ != union_block->cases.size() - 1)
				os << '\n';
		}

		os << tabs << "}\n";
	}
};

void Printer::print_serialize_code(std::ostream& os, const DataBlockEntries& dbe,
                                   int depth, const std::string& prefix) const
{
	auto tabs = make_tabs(depth);

	for (auto& entry : dbe.entries)
	{
		std::visit(serialize_print_visitor{*this, dbe, os, m_proto, depth, tabs, prefix}, entry);
	}
}

struct unserialize_print_visitor
{
	const Printer& printer;
	const DataBlockEntries& dbe;
	std::ostream& os;
	const ProtocolFile& proto;
	const int depth;
	const std::string& tabs;
	const std::string& prefix;

	struct wrap_result_t
	{
		std::function<void()> print_prefix;
		std::function<void()> print_suffix;
		std::function<std::string(const std::string&)> wrap_id;
		std::string tabs;
	};

	wrap_result_t wrap_field(std::optional<int> static_size,
	                         std::optional<std::string> dynamic_size,
	                         bool implicit_size, std::optional<std::string> name)
	{
		wrap_result_t result;

		result.print_prefix = []() { };
		result.print_suffix = []() { };
		result.wrap_id = [](const std::string& s) { return s; };

		std::optional<std::string> loop_cond;
		std::string loop_idx;
		bool known_size = static_size || (dynamic_size && name);

		if (static_size)
			loop_cond = "i < " + std::to_string(static_size.value());
		else if (dynamic_size)
			loop_cond = "i < " + dynamic_size.value();
		else if (implicit_size)
			loop_cond = "reader.unbroken()";

		if (loop_cond)
		{
			result.tabs = make_tabs(depth + 1);

			result.print_prefix = [=]()
			{
				if (known_size && !static_size)
				{
					os << tabs << prefix << name.value() << ".resize("
					   << prefix << dynamic_size.value() << ");\n";
				}

				os << tabs << "for (std::size_t i = 0; "
				   << loop_cond.value()  << "; ++i)\n";

				if (!known_size)
					os << tabs << "{\n";
			};

			result.wrap_id = [=](const std::string& s)
			{
				if (known_size)
					return s + "[i]";
				else
					return s + ".push_back({});\n " + result.tabs + s + ".back()";
			};

			if (!known_size)
			{
				result.print_suffix = [this, loop_cond]()
				{
					os << tabs << "}\n";
				};
			}
		}
		else
		{
			result.tabs = tabs;
		}

		return result;
	}

	void operator()(const std::shared_ptr<DataField>& data_field)
	{
		auto wrap = wrap_field(data_field->static_size, data_field->dynamic_size,
		                       data_field->implicit_size, data_field->name);

		std::string base_type = data_field->type;

		if (data_field->type_class)
			base_type = data_field->type_class.value();

		base_type = printer.enum_base_type(base_type);

		std::string get = printer.get_fn(base_type) + "()";

		std::string cast_begin;
		std::string cast_end;

		if (data_field->type_static_size)
			get = "get_fixed_string(" + std::to_string(data_field->type_static_size.value()) + ")";
		else if (data_field->type_dynamic_size)
			get = "get_fixed_string(" + prefix + data_field->type_dynamic_size.value() + ")";

		if (printer.type_type(data_field->type) == Printer::type_enum)
		{
			cast_begin = "static_cast<" + data_field->type + ">(";
			cast_end = ")";
		}

		wrap.print_prefix();

		if (data_field->name)
		{
			os << wrap.tabs << wrap.wrap_id(prefix + data_field->name.value())
			   << " = " << cast_begin << "reader." << get << cast_end << ";\n";
		}
		else
		{
			os << wrap.tabs << "reader." << get << ";\n";
		}

		wrap.print_suffix();
	}

	void operator()(const std::shared_ptr<StructField>& struct_field)
	{
		auto wrap = wrap_field(struct_field->static_size, struct_field->dynamic_size,
		                       struct_field->implicit_size, struct_field->name);

		wrap.print_prefix();

		os << wrap.tabs << wrap.wrap_id(prefix + struct_field->name.value())
		   << ".unserialize(reader);\n";

		wrap.print_suffix();
	}

	void operator()(const std::shared_ptr<UnionBlock>& union_block)
	{
		os << '\n';

		os << tabs << "switch (" << prefix << union_block->switch_field << ")\n"
		   << tabs << "{\n";

		std::size_t i = 0;

		auto enum_type = printer.get_enum_type(dbe, union_block->switch_field);

		for (const auto& case_it : union_block->cases)
		{
			auto& case_value = case_it.first;
			auto& case_data = *case_it.second;

			std::string new_prefix = prefix + "u." + case_data.name + ".";

			if (case_value == "default")
				os << tabs << "\tdefault:\n";
			else
				os << tabs << "\tcase " << enum_type << "::" << case_value << ":\n";

			os << tabs << "\t\tnew(&" << prefix << "u." << case_data.name << ") "
			      "decltype(" << prefix << "u." << case_data.name << ");\n";

			printer.print_unserialize_code(os, case_data.dbe, depth+2, new_prefix);

			os << tabs << "\tbreak;\n";

			if (i++ != union_block->cases.size() - 1)
				os << '\n';
		}

		os << tabs << "}\n";
	}
};

void Printer::print_unserialize_code(std::ostream& os, const DataBlockEntries& dbe,
                                     int depth, const std::string& prefix) const
{
	auto tabs = make_tabs(depth);

	for (auto& entry : dbe.entries)
	{
		std::visit(unserialize_print_visitor{*this, dbe, os, m_proto, depth, tabs, prefix}, entry);
	}
}

void Printer::print_enum(std::ostream& os,
                         const std::pair<std::string, EnumBlock::ptr>& enum_it,
                         int depth) const
{
	auto tabs = make_tabs(depth);

	auto& enum_name = enum_it.first;
	auto& enum_data = *enum_it.second;

	if (enum_data.comment)
		os << tabs << "// " << enum_data.comment.value() << "\n";

	auto enum_type = enum_data.type_class.value_or("int");

	os << tabs << "enum class " << enum_name << " : " << map_type(enum_type) << "\n"
	   << tabs << "{\n";

	for (auto& enum_value_it : make_sorted_by_value(enum_data.entries))
	{
		auto& entry_name = enum_value_it.first;
		auto& entry_value = enum_value_it.second;

		os << tabs << "\t" << entry_name << " = " << entry_value << ",\n";
	}

	os << tabs << "};\n";
}

void Printer::print_struct(std::ostream& os,
                           const std::pair<std::string, StructBlock::ptr>& struct_it,
                           int depth) const
{
	auto tabs = make_tabs(depth);

	auto& struct_name = struct_it.first;
	auto& struct_data = *struct_it.second;

	if (struct_data.comment)
		os << tabs << "// " << struct_data.comment.value() << "\n";

	os << tabs << "struct " << struct_name << "\n{\n";

	print_data_block(os, struct_data.dbe, depth + 1);

	os << '\n'
	   << tabs << "\t" << struct_name << "() = default;\n"
	   << tabs << "\t" << struct_name << "(EO_Stream_Reader& reader) { unserialize(reader); }\n"
	   << tabs << "\tstd::size_t byte_size() const;\n"
	   << tabs << "\tvoid serialize(EO_Stream_Builder& builder) const;\n"
	   << tabs << "\tvoid unserialize(EO_Stream_Reader& reader);\n"
	   << tabs << "};\n";
}

void Printer::print_struct_impl(std::ostream& os,
                                const std::pair<std::string, StructBlock::ptr>& struct_it,
                                int depth) const
{
	auto tabs = make_tabs(depth);

	auto& struct_name = struct_it.first;
	auto& struct_data = *struct_it.second;

	os << tabs << "std::size_t " << struct_name << "::byte_size() const\n"
	   << tabs << "{\n"
	   << tabs << "\treturn " << make_byte_size_expression(struct_data.dbe) << ";\n"
	   << tabs << "}\n\n";

	os << tabs << "void " << struct_name << "::serialize(EO_Stream_Builder& builder) const\n"
	   << tabs << "{\n";

	print_serialize_code(os, struct_data.dbe, depth + 1);

	os << tabs << "}\n\n";

	os << tabs << "void " << struct_name << "::unserialize(EO_Stream_Reader& reader)\n"
	   << tabs << "{\n";

	print_unserialize_code(os, struct_data.dbe, depth + 1);

	os << tabs << "}\n";
}

void Printer::print_client_def(std::ostream& os,
                              const std::pair<std::string, PacketBlock::ptr>& packet_it,
                              int depth) const
{
	auto tabs = make_tabs(depth);

	auto& packet_name = packet_it.first;
	auto& packet_data = *packet_it.second;

	if (packet_data.comment)
		os << tabs << "// " << packet_data.comment.value() << "\n";

	os << tabs << "struct " << packet_name << " : public Client_Packet\n"
	   << tabs << "{\n"
	   << tabs << "\tstatic constexpr PacketFamily family = PacketFamily::" << packet_data.family << ";\n"
	   << tabs << "\tstatic constexpr PacketAction action = PacketAction::" << packet_data.action << ";\n"
	   << tabs << "\tstatic constexpr PacketID id = {family, action};\n"
	   << tabs << '\n';

	print_data_block(os, packet_data.dbe, depth + 1);

	os << '\n'
	   << tabs << "\tvirtual ~" << packet_name << "() override final = default;\n"
	   << tabs << "\tvirtual std::size_t byte_size() const override final;\n"
	   << tabs << "\tvirtual void serialize(EO_Stream_Builder& builder) const override final;\n"
	   << tabs << "\tvirtual PacketID vid() const override final;\n"
	   << tabs << "};\n";

}

void Printer::print_server_def(std::ostream& os,
                              const std::pair<std::string, PacketBlock::ptr>& packet_it,
                              int depth) const
{
	auto tabs = make_tabs(depth);

	auto& packet_name = packet_it.first;
	auto& packet_data = *packet_it.second;

	if (packet_data.comment)
		os << tabs << "// " << packet_data.comment.value() << "\n";

	os << tabs << "struct " << packet_name << " : public Server_Packet\n"
	   << tabs << "{\n"
	   << tabs << "\tstatic constexpr PacketFamily family = PacketFamily::" << packet_data.family << ";\n"
	   << tabs << "\tstatic constexpr PacketAction action = PacketAction::" << packet_data.action << ";\n"
	   << tabs << "\tstatic constexpr PacketID id = {family, action};\n"
	   << tabs << '\n';

	print_data_block(os, packet_data.dbe, depth + 1);

	os << '\n'
	   << tabs << "\t" << packet_name << "() = default;\n"
	   << tabs << "\t" << packet_name << "(EO_Stream_Reader& reader) { unserialize(reader); }\n"
	   << tabs << "\tvirtual ~" << packet_name << "() override final = default;\n"
	   << tabs << "\tvirtual void unserialize(EO_Stream_Reader& reader) override final;\n"
	   << tabs << "\tvirtual PacketID vid() const override final;\n"
	   << tabs << "};\n";
}

void Printer::print_client_impl(std::ostream& os,
                                const std::pair<std::string, PacketBlock::ptr>& packet_it,
                                int depth) const
{
	auto tabs = make_tabs(depth);

	auto& packet_name = packet_it.first;
	auto& packet_data = *packet_it.second;

	os << tabs << "std::size_t " << packet_name << "::byte_size() const\n"
	   << tabs << "{\n"
	   << tabs << "\treturn " << make_byte_size_expression(packet_data.dbe) << ";\n"
	   << tabs << "}\n\n";

	os << tabs << "void " << packet_name << "::serialize(EO_Stream_Builder& builder) const\n"
	   << tabs << "{\n";

	print_serialize_code(os, packet_data.dbe, depth + 1);

	os << tabs << "}\n\n";

	os << tabs << "constexpr PacketID " << packet_name << "::id;\n";

	os << tabs << "PacketID " << packet_name << "::vid() const\n"
	   << tabs << "{ return id; }\n";
}

void Printer::print_server_impl(std::ostream& os,
                                const std::pair<std::string, PacketBlock::ptr>& packet_it,
                                int depth) const
{
	auto tabs = make_tabs(depth);

	auto& packet_name = packet_it.first;
	auto& packet_data = *packet_it.second;

	os << tabs << "void " << packet_name << "::unserialize(EO_Stream_Reader& reader)\n"
	   << tabs << "{\n";

	print_unserialize_code(os, packet_data.dbe, depth + 1);

	os << tabs << "}\n\n";

	os << tabs << "constexpr PacketID " << packet_name << "::id;\n";

	os << tabs << "PacketID " << packet_name << "::vid() const\n"
	   << tabs << "{ return id; }\n";
}

void Printer::print_enums(std::ostream& os, int depth) const
{
	for (auto& enum_it : make_sorted_by_key(m_proto.enums))
	{
		print_enum(os, enum_it, depth);
		os << "\n";
	}
}

void Printer::print_structs(std::ostream& os, int depth) const
{
	auto structs_sorted = make_sorted_by_key(m_proto.structs);
	using vec = decltype(structs_sorted);

	auto do_sort = [](vec& from, vec& to)
	{
		int moves = 0;

		auto from_copy = from;

		for (auto& struct_it : from_copy)
		{
			std::function<void(const std::deque<PacketBlockEntry>& entries)>
				check_entries = [&](const std::deque<PacketBlockEntry>& entries)
			{
				for (auto& entry_it : entries)
				{
					if (entry_it.index() == 0) // data field
						continue;

					if (entry_it.index() == 2) // union field
					{
						auto& block = std::get<UnionBlock::ptr>(entry_it);

						for (auto& case_it : block->cases)
							check_entries(case_it.second->dbe.entries);

						return;
					}

					auto& field = std::get<StructField::ptr>(entry_it);
					auto& type = field->type;

					auto find_it = std::find_if(from.begin(), from.end(),
						[&](decltype(struct_it) check_struct_it)
						{
							auto& check_name = check_struct_it.first;
							return (check_name == type);
						}
					);

					if (find_it != from.end())
					{
						++moves;
						to.push_back(*find_it);
						from.erase(find_it);
					}
				}
			};

			check_entries(struct_it.second->dbe.entries);
		}

		return moves;
	};

	// Try order based on dependency
	{
		std::list<vec> structs_list = {};

		vec remaining = structs_sorted;

		structs_sorted.clear();

		int moves;

		do
		{
			vec head;
			moves = do_sort(remaining, head);
			structs_list.push_front(remaining);
			remaining = head;
		}
		while (moves > 0);

		for (auto& structs : structs_list)
			for (auto& struct_it : structs)
				structs_sorted.push_back(struct_it);
	}

	for (auto& struct_it : structs_sorted)
	{
		print_struct(os, struct_it, depth);
		os << "\n";
	}
}

void Printer::print_struct_impls(std::ostream& os, int depth) const
{
	for (auto& struct_it : make_sorted_by_key(m_proto.structs))
	{
		print_struct_impl(os, struct_it, depth);
		os << "\n";
	}
}

void Printer::print_client_packet_defs(std::ostream& os, int depth) const
{
	auto packets_sorted = make_sorted_by_key(m_proto.client_packets);

	for (auto& packet_it : packets_sorted)
	{
		print_client_def(os, packet_it, depth);
		os << "\n";
	}
}

void Printer::print_server_packet_defs(std::ostream& os, int depth) const
{
	auto packets_sorted = make_sorted_by_key(m_proto.server_packets);

	for (auto& packet_it : packets_sorted)
	{
		print_server_def(os, packet_it, depth);
		os << "\n";
	}
}

void Printer::print_client_packet_variant(std::ostream& os, int depth) const
{
	auto packets_sorted = make_sorted_by_key(m_proto.client_packets);

	os << "\nusing Client_Packet = jss::variant<\n";

	std::size_t i = 0;

	for (auto& packet_it : packets_sorted)
	{
		os << "\tclient::" << packet_it.first;

		if (i++ != packets_sorted.size() - 1)
			os << ',';

		os << "\n";
	}

	os << ">;\n\n";
}

void Printer::print_server_packet_variant(std::ostream& os, int depth) const
{
	auto packets_sorted = make_sorted_by_key(m_proto.server_packets);

	os << "\nusing Server_Packet = jss::variant<\n";

	std::size_t i = 0;

	for (auto& packet_it : packets_sorted)
	{
		os << "\tserver::" << packet_it.first;

		if (i++ != packets_sorted.size() - 1)
			os << ',';

		os << "\n";
	}

	os << ">;\n\n";
}

void Printer::print_client_packet_cases(std::ostream& os, int depth) const
{
	auto packets_sorted = make_sorted_by_key(m_proto.client_packets);

	for (auto& packet_it : packets_sorted)
	{
		os << "\tcase_packet(" << packet_it.first << ")\n";
	}
}

void Printer::print_server_packet_cases(std::ostream& os, int depth) const
{
	auto packets_sorted = make_sorted_by_key(m_proto.server_packets);

	for (auto& packet_it : packets_sorted)
	{
		os << "\tcase_packet(" << packet_it.first << ")\n";
	}
}

void Printer::print_client_packet_impls(std::ostream& os, int depth) const
{
	for (auto& packet_it : make_sorted_by_key(m_proto.client_packets))
	{
		print_client_impl(os, packet_it, depth);
		os << "\n";
	}
}

void Printer::print_server_packet_impls(std::ostream& os, int depth) const
{
	for (auto& packet_it : make_sorted_by_key(m_proto.server_packets))
	{
		print_server_impl(os, packet_it, depth);
		os << "\n";
	}
}
