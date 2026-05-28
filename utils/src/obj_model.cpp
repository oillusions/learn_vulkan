#include "utils/obj_model.hpp"
#include "utils/global_logger.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <istream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

const fs::path test_path = "./resource/model/model_default.obj";

inline std::expected<std::vector<char>, Error> 
read_model(const fs::path& model_path) {
    if (!fs::exists(test_path)) return Error(
        std::format("模型{}不存在", test_path.filename().string())
    );

    std::ifstream model_file(test_path, std::ios::ate);
    if (!model_file.is_open()) return Error(
        std::format("模型{}打开失败", test_path.filename().string())
    );

    std::vector<char> content;

    const size_t file_content_size = model_file.tellg();
    if (file_content_size == 0) return Error(
        std::format("模型{}为空", test_path.filename().string())
    );

    content.resize(file_content_size);

    model_file.seekg(0);

    model_file.read(content.data(), file_content_size);

    return content;
}

std::vector<std::string> 
split(std::string& content, std::string_view delim) noexcept {
    while (true) {
        size_t pos = content.find('\n');
        if (pos != std::string_view::npos) {
            content[pos] = ' ';
        } else {
            break;
        }
    }

    std::vector<std::string> ret;
    size_t start = 0;
    size_t end = content.find(delim);
    while (end != std::string_view::npos) {
        if (start != end) {
            ret.emplace_back(content.substr(start, end - start));
        }
        start = end + 1;
        end = content.find(delim, start);
    }
    if (start < content.size()) {
        ret.emplace_back(content.substr(start, end - start));
    }
    return ret;
}

enum class Key {
    Unknown = 0,

    Vertex,
    TexCoord,
    Normal,
    Pixel,
    Line,
    Face,
    Object,
    Group
};

inline Key match_key(std::string_view token) noexcept {
    if (token == "v") return Key::Vertex;
    if (token == "vt") return Key::TexCoord;
    if (token == "vn") return Key::Normal;

    if (token == "p") return Key::Pixel;
    if (token == "l") return Key::Line;
    if (token == "f") return Key::Face;

    if (token == "o") return Key::Object;
    if (token == "g") return Key::Group;

    return Key::Unknown;
}

std::string to_string(const Key key) noexcept {
    switch (key) {
        case Key::Unknown: {
            return "unknown";
        }
        case Key::Vertex: {
            return "vertex";
        }
        case Key::TexCoord: {
            return "tex_coord";
        }
        case Key::Normal: {
            return "normal";
        }
        case Key::Pixel: {
            return "pixel";
        }
        case Key::Line: {
            return "line";
        }
        case Key::Face: {
            return "face";
        }
        case Key::Object: {
            return "object";
        }
        case Key::Group: {
            return "group";
        }
    }
}

ObjModel::ObjModel(
    std::string name,
    ObjModel::ModelResource resource
) noexcept :
    name(std::move(name)),
    resource(std::move(resource))
{}

template<typename T>
std::string to_string(const T&) noexcept;

template<>
std::string to_string(const glm::vec2& vec) noexcept {
    return std::format("vec2({}, {})", 
        std::to_string(vec.x), 
        std::to_string(vec.y)
    );
}

template<>
std::string to_string(const glm::vec3& vec) noexcept {
    return std::format("vec3({}, {}, {})", 
        std::to_string(vec.x), 
        std::to_string(vec.y),
        std::to_string(vec.z)
    );
}

inline std::expected<ObjModel, Error> 
obj_parser(const std::vector<std::string>& tokens, size_t& index) noexcept {
    std::vector<float> vertices;
    std::vector<float> tex_coords;

    std::vector<std::string> indices;

    const auto& name = tokens[index];
    index++;

    Key curr_key{};
    while (index != tokens.size()) {
        glog.log<LogLevel::Debug>(std::format("t[{}]:{}", to_string(curr_key), tokens[index]));
        switch (curr_key) {
            case Key::Unknown: {
                curr_key = match_key(tokens[index]);
                index++;
                break;
            }
            case Key::Vertex: {
                if (match_key(tokens[index]) == Key::Unknown) {
                    vertices.push_back(std::stof(tokens[index]));
                } else {
                    curr_key = match_key(tokens[index]);
                }
                index++;
                break;
            }
            case Key::TexCoord: {
                if (match_key(tokens[index]) == Key::Unknown) {
                    tex_coords.push_back(std::stof(tokens[index]));
                } else {
                    curr_key = match_key(tokens[index]);
                }
                index++;
                break;
            }
            case Key::Normal: {
                index++;
                break;
            }
            case Key::Face: {
                if (match_key(tokens[index]) == Key::Unknown) {
                    indices.push_back(tokens[index]);
                } else {
                    curr_key = match_key(tokens[index]);
                }
                index++;
                break;
            }
            case Key::Object: {
                index++;
                break;
            }
            default:;
        }
    }

    if (vertices.empty()) return Error("顶点为空");
    if (tex_coords.empty()) return Error("纹理坐标为空");
    if (indices.empty()) return Error("索引为空");

    std::vector<ObjModel::Vertex> ret_vertices;
    ret_vertices.reserve(indices.size());

    std::vector<uint32_t> ret_indices;
    ret_indices.reserve(indices.size());

    std::vector<float> temp;
    temp.resize(6);

    size_t count{0};
    for (auto& f_index_str : indices) {
        const auto f_indexs = split(f_index_str, "/");
        const auto v_index = (std::stoul(f_indexs[0]) - 1) * 3;
        const auto t_index = (std::stoul(f_indexs[1]) - 1) * 2;

        glog.log<LogLevel::Debug>(std::format("v:{}, t:{}", v_index, t_index));

        ret_indices.push_back(count);
        ret_vertices.push_back(
            ObjModel::Vertex{
                glm::vec3(
                    vertices[v_index],
                    vertices[v_index + 1],
                    vertices[v_index + 2]
                ),
                glm::vec2(
                    tex_coords[t_index],
                    tex_coords[t_index + 1]
                )
            }
        );
        count++;
    }

    for (const auto& vertex : ret_vertices) {
        glog.log<LogLevel::Debug>(std::format("v:{}, t:{}", to_string(vertex.vertex), to_string(vertex.tex_coord)));
    }

    return ObjModel(
        name,
        ObjModel::ModelResource{
            std::move(ret_vertices),
            std::move(ret_indices)
        }
    );
}

vk::VertexInputBindingDescription 
ObjModel::obtain_vertex_bind_description() noexcept {
    return vk::VertexInputBindingDescription()
        .setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(sizeof(ObjModel::Vertex));
}

std::vector<vk::VertexInputAttributeDescription> 
ObjModel::obtain_vertex_attribute_description() noexcept {
    return std::vector {
        vk::VertexInputAttributeDescription()
            .setBinding(0)
            .setFormat(vk::Format::eR32G32B32Sfloat)
            .setLocation(0)
            .setOffset(offsetof(ObjModel::Vertex, vertex)),
        vk::VertexInputAttributeDescription()
            .setBinding(0)
            .setFormat(vk::Format::eR32G32Sfloat)
            .setLocation(1)
            .setOffset(offsetof(ObjModel::Vertex, tex_coord)),
    };
}

vk::PipelineInputAssemblyStateCreateInfo 
ObjModel::obtain_input_assembly_info() noexcept {
    return vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(vk::False);
}

std::expected<std::vector<ObjModel>, Error> 
ObjModel::create() noexcept {
    auto result_model_content = read_model(test_path);
    if (!result_model_content) return result_model_content.error().forward("加载模型失败");
    auto content = std::string(
        result_model_content.value().data(), 
        result_model_content.value().size()
    );

    auto tokens = split(content, " ");

    std::vector<ObjModel> ret;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (match_key(tokens[i]) == Key::Object) {
            i++;
            auto result_model = obj_parser(tokens, i);
            if (!result_model) return result_model.error();
            ret.emplace_back(std::move(result_model.value()));
        }
    }

    if (ret.empty()) return Error("无可用模型");

    return ret;
}