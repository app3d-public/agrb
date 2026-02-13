#include <args.hxx>
#include <acul/io/fs/file.hpp>
#include <acul/io/fs/path.hpp>
#include <acul/string/utils.hpp>
#include <agrb/agrb.hpp>
#include <agrb/pipeline.hpp>
#include <umbf/umbf.hpp>
#include <umbf/version.h>

namespace
{
    struct config
    {
        acul::string input_dir;
        acul::string output_file;
        int compression = 5;
    };

    acul::string get_stem(const acul::string &file_name) { return acul::fs::replace_extension(file_name, ""); }

    struct shader_entry
    {
        u64 id;
        acul::string path;
    };

    bool parse_args(int argc, char **argv, config &cfg)
    {
        args::ArgumentParser parser("Shaders to UMBF library packer", "");
        args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
        args::ValueFlag<std::string> input(parser, "dir", "Input directory with *.spv files", {'i', "input"});
        args::ValueFlag<std::string> output(parser, "file", "Output umbf library file (*.umlib)", {'o', "output"});
        args::ValueFlag<int> compression(parser, "level", "Compression level [0..22]", {'c', "compression"});

        try
        {
            parser.ParseCLI(argc, argv);
        }
        catch (const args::Help &)
        {
            std::printf("%s", parser.Help().c_str());
            return false;
        }
        catch (const args::ParseError &e)
        {
            std::fprintf(stderr, "%s\n%s", e.what(), parser.Help().c_str());
            return false;
        }
        catch (const args::ValidationError &e)
        {
            std::fprintf(stderr, "%s\n%s", e.what(), parser.Help().c_str());
            return false;
        }

        if (!input || !output)
        {
            std::fprintf(stderr, "Both --input and --output are required\n%s", parser.Help().c_str());
            return false;
        }

        cfg.input_dir = args::get(input).c_str();
        cfg.output_file = args::get(output).c_str();
        if (compression) cfg.compression = args::get(compression);

        if (cfg.compression < 0 || cfg.compression > 22)
        {
            std::fprintf(stderr, "Compression must be in range [0..22]\n");
            return false;
        }

        return true;
    }

    bool collect_entries(const acul::string &input_dir, acul::vector<shader_entry> &entries)
    {
        acul::vector<acul::string> files;
        auto lr = acul::fs::list_files(input_dir, files, false);
        if (!lr.success())
        {
            std::fprintf(stderr, "Failed to list files in '%s' (error=0x%016llx)\n", input_dir.c_str(),
                         static_cast<unsigned long long>(static_cast<u64>(lr)));
            return false;
        }

        acul::hashset<u64> unique_ids;
        for (const auto &path : files)
        {
            if (acul::fs::get_extension(path) != ".spv") continue;

            const acul::string file_name = acul::fs::get_filename(path);
            const acul::string stem = get_stem(file_name);
            const char *cursor = stem.c_str();
            unsigned long long parsed = 0;
            if (!acul::stoull_hex(cursor, parsed) || *cursor != '\0')
            {
                std::fprintf(stderr, "Skipping '%s': filename stem must be final hex shader ID\n", file_name.c_str());
                continue;
            }
            const u64 id = static_cast<u64>(parsed);

            if (!unique_ids.insert(id).second)
            {
                std::fprintf(stderr, "Duplicate shader id 0x%016llX from '%s'\n", static_cast<unsigned long long>(id),
                             file_name.c_str());
                return false;
            }

            entries.push_back({id, path});
        }
        return true;
    }
} // namespace

int main(int argc, char **argv)
{
    config cfg;
    if (!parse_args(argc, argv, cfg)) return 2;

    agrb::init_library();

    acul::vector<shader_entry> entries;
    if (!collect_entries(cfg.input_dir, entries))
    {
        agrb::destroy_library();
        return 2;
    }

    if (entries.empty())
    {
        std::fprintf(stderr, "No .spv files found in '%s'\n", cfg.input_dir.c_str());
        agrb::destroy_library();
        return 2;
    }

    auto library = acul::make_shared<umbf::Library>();
    library->file_tree.is_folder = true;

    for (const auto &entry : entries)
    {
        acul::vector<char> bytes;
        if (!acul::fs::read_binary(entry.path, bytes))
        {
            std::fprintf(stderr, "Failed to read shader binary '%s'\n", entry.path.c_str());
            agrb::destroy_library();
            return 2;
        }

        auto block = acul::make_shared<agrb::shader_block>();
        block->id = entry.id;
        block->code = std::move(bytes);

        umbf::Library::Node shader_node;
        shader_node.name = acul::fs::get_filename(entry.path);
        shader_node.is_folder = false;
        shader_node.asset.header.vendor_sign = AGRB_VENDOR_ID;
        shader_node.asset.header.vendor_version = AGRB_VERSION;
        shader_node.asset.header.spec_version = UMBF_VERSION;
        shader_node.asset.header.type_sign = AGRB_TYPE_ID_SHADER;
        shader_node.asset.header.compressed = false;
        shader_node.asset.blocks.push_back(acul::static_pointer_cast<umbf::Block>(block));
        library->file_tree.children.push_back(std::move(shader_node));
    }

    umbf::File file;
    file.header.vendor_sign = UMBF_VENDOR_ID;
    file.header.vendor_version = UMBF_VERSION;
    file.header.spec_version = UMBF_VERSION;
    file.header.type_sign = umbf::sign_block::format::library;
    file.header.compressed = cfg.compression > 0;
    file.blocks.push_back(library);

    umbf::streams::HashResolver resolver;
    resolver.streams.emplace(static_cast<u32>(umbf::sign_block::library), &umbf::streams::library);
    resolver.streams.emplace(static_cast<u32>(AGRB_TYPE_ID_SHADER), &agrb::streams::shader);
    resolver.streams.emplace(static_cast<u32>(AGRB_SIGN_ID_SHADER), &agrb::streams::shader);
    umbf::streams::resolver = &resolver;

    const bool ok = file.save(cfg.output_file, cfg.compression);

    agrb::destroy_library();

    if (!ok)
    {
        std::fprintf(stderr, "Failed to save umbf file '%s'\n", cfg.output_file.c_str());
        return 2;
    }

    std::printf("Packed %zu shaders into %s\n", static_cast<size_t>(entries.size()), cfg.output_file.c_str());
    return 0;
}
