#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

#include "log.h"
#include "xdl.h"
#include "il2cpp-api-functions.h"
#include "il2cpp-class.h"
#include "il2cpp-tabledefs.h"

static uintptr_t il2cpp_base = 0;

// ===================== utils =====================
uintptr_t get_module_base(const char *name) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, name)) {
            uintptr_t base = strtoull(line, nullptr, 16);
            fclose(fp);
            return base;
        }
    }
    fclose(fp);
    return 0;
}

// ===================== api init =====================
void init_il2cpp() {
    void *handle = xdl_open("libil2cpp.so", XDL_DEFAULT);
    if (!handle) {
        LOGE("xdl_open libil2cpp.so failed");
        return;
    }

#define DO_API(r, n, p) \
    n = (r (*) p)xdl_sym(handle, #n, nullptr); \
    if (!n) LOGW("missing api: %s", #n);

#include "il2cpp-api-functions.h"
#undef DO_API

    il2cpp_base = get_module_base("libil2cpp.so");
    LOGI("il2cpp_base = %p", (void *)il2cpp_base);

    while (!il2cpp_domain_get()) {
        LOGI("waiting for il2cpp_domain_get...");
        sleep(1);
    }

    il2cpp_thread_attach(il2cpp_domain_get());
}

// ===================== dump helpers =====================
std::string method_flags(uint32_t flags) {
    std::stringstream ss;
    if (flags & METHOD_ATTRIBUTE_PUBLIC) ss << "public ";
    if (flags & METHOD_ATTRIBUTE_PRIVATE) ss << "private ";
    if (flags & METHOD_ATTRIBUTE_FAMILY) ss << "protected ";
    if (flags & METHOD_ATTRIBUTE_STATIC) ss << "static ";
    if (flags & METHOD_ATTRIBUTE_ABSTRACT) ss << "abstract ";
    if (flags & METHOD_ATTRIBUTE_VIRTUAL) ss << "virtual ";
    return ss.str();
}

std::string dump_methods(Il2CppClass *klass) {
    std::stringstream ss;
    void *iter = nullptr;
    while (auto method = il2cpp_class_get_methods(klass, &iter)) {
        uintptr_t va = (uintptr_t)method->methodPointer;
        uintptr_t rva = va > il2cpp_base ? (va - il2cpp_base) : 0;

        ss << "\n\t// RVA: 0x" << std::hex << rva
           << " VA: 0x" << va << "\n\t";

        uint32_t iflags;
        auto flags = il2cpp_method_get_flags(method, &iflags);
        ss << method_flags(flags);

        auto ret = il2cpp_method_get_return_type(method);
        auto retClass = il2cpp_class_from_type(ret);
        ss << il2cpp_class_get_name(retClass) << " "
           << il2cpp_method_get_name(method) << "(";

        int pc = il2cpp_method_get_param_count(method);
        for (int i = 0; i < pc; i++) {
            auto p = il2cpp_method_get_param(method, i);
            auto pcClass = il2cpp_class_from_type(p);
            ss << il2cpp_class_get_name(pcClass) << " "
               << il2cpp_method_get_param_name(method, i);
            if (i + 1 < pc) ss << ", ";
        }
        ss << ") { }\n";
    }
    return ss.str();
}

std::string dump_fields(Il2CppClass *klass) {
    std::stringstream ss;
    void *iter = nullptr;
    while (auto field = il2cpp_class_get_fields(klass, &iter)) {
        auto type = il2cpp_field_get_type(field);
        auto fClass = il2cpp_class_from_type(type);
        ss << "\t" << il2cpp_class_get_name(fClass)
           << " " << il2cpp_field_get_name(field)
           << "; // 0x" << std::hex
           << il2cpp_field_get_offset(field) << "\n";
    }
    return ss.str();
}

std::string dump_class(Il2CppClass *klass) {
    std::stringstream ss;
    ss << "\n// Namespace: "
       << il2cpp_class_get_namespace(klass) << "\n";
    ss << "class " << il2cpp_class_get_name(klass) << "\n{\n";
    ss << dump_fields(klass);
    ss << dump_methods(klass);
    ss << "}\n";
    return ss.str();
}

// ===================== main dump =====================
void dump_il2cpp(const char *outPath) {
    init_il2cpp();

    auto domain = il2cpp_domain_get();
    size_t count = 0;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &count);

    std::ofstream out(outPath);
    if (!out.is_open()) {
        LOGE("open dump file failed");
        return;
    }

    // ⚠️ UNITY 2022: DÙNG REFLECTION
    auto corlib = il2cpp_get_corlib();
    auto asmClass = il2cpp_class_from_name(
        corlib, "System.Reflection", "Assembly");

    auto load = il2cpp_class_get_method_from_name(asmClass, "Load", 1);
    auto getTypes = il2cpp_class_get_method_from_name(asmClass, "GetTypes", 0);

    typedef void *(*Assembly_Load)(void *, Il2CppString *, void *);
    typedef Il2CppArray *(*Assembly_GetTypes)(void *, void *);

    for (int i = 0; i < count; i++) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        auto name = il2cpp_image_get_name(image);
        out << "\n// Dll: " << name << "\n";

        auto dot = std::string(name).rfind('.');
        auto shortName = std::string(name).substr(0, dot);
        auto str = il2cpp_string_new(shortName.c_str());

        auto asmObj = ((Assembly_Load)load->methodPointer)(nullptr, str, nullptr);
        auto types = ((Assembly_GetTypes)getTypes->methodPointer)(asmObj, nullptr);

        for (int j = 0; j < types->max_length; j++) {
            auto klass = il2cpp_class_from_system_type(
                (#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

#include "log.h"
#include "xdl.h"
#include "il2cpp-api-functions.h"
#include "il2cpp-class.h"
#include "il2cpp-tabledefs.h"

static uintptr_t il2cpp_base = 0;

// ===================== utils =====================
uintptr_t get_module_base(const char *name) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, name)) {
            uintptr_t base = strtoull(line, nullptr, 16);
            fclose(fp);
            return base;
        }
    }
    fclose(fp);
    return 0;
}

// ===================== api init =====================
void init_il2cpp() {
    void *handle = xdl_open("libil2cpp.so", XDL_DEFAULT);
    if (!handle) {
        LOGE("xdl_open libil2cpp.so failed");
        return;
    }

#define DO_API(r, n, p) \
    n = (r (*) p)xdl_sym(handle, #n, nullptr); \
    if (!n) LOGW("missing api: %s", #n);

#include "il2cpp-api-functions.h"
#undef DO_API

    il2cpp_base = get_module_base("libil2cpp.so");
    LOGI("il2cpp_base = %p", (void *)il2cpp_base);

    while (!il2cpp_domain_get()) {
        LOGI("waiting for il2cpp_domain_get...");
        sleep(1);
    }

    il2cpp_thread_attach(il2cpp_domain_get());
}

// ===================== dump helpers =====================
std::string method_flags(uint32_t flags) {
    std::stringstream ss;
    if (flags & METHOD_ATTRIBUTE_PUBLIC) ss << "public ";
    if (flags & METHOD_ATTRIBUTE_PRIVATE) ss << "private ";
    if (flags & METHOD_ATTRIBUTE_FAMILY) ss << "protected ";
    if (flags & METHOD_ATTRIBUTE_STATIC) ss << "static ";
    if (flags & METHOD_ATTRIBUTE_ABSTRACT) ss << "abstract ";
    if (flags & METHOD_ATTRIBUTE_VIRTUAL) ss << "virtual ";
    return ss.str();
}

std::string dump_methods(Il2CppClass *klass) {
    std::stringstream ss;
    void *iter = nullptr;
    while (auto method = il2cpp_class_get_methods(klass, &iter)) {
        uintptr_t va = (uintptr_t)method->methodPointer;
        uintptr_t rva = va > il2cpp_base ? (va - il2cpp_base) : 0;

        ss << "\n\t// RVA: 0x" << std::hex << rva
           << " VA: 0x" << va << "\n\t";

        uint32_t iflags;
        auto flags = il2cpp_method_get_flags(method, &iflags);
        ss << method_flags(flags);

        auto ret = il2cpp_method_get_return_type(method);
        auto retClass = il2cpp_class_from_type(ret);
        ss << il2cpp_class_get_name(retClass) << " "
           << il2cpp_method_get_name(method) << "(";

        int pc = il2cpp_method_get_param_count(method);
        for (int i = 0; i < pc; i++) {
            auto p = il2cpp_method_get_param(method, i);
            auto pcClass = il2cpp_class_from_type(p);
            ss << il2cpp_class_get_name(pcClass) << " "
               << il2cpp_method_get_param_name(method, i);
            if (i + 1 < pc) ss << ", ";
        }
        ss << ") { }\n";
    }
    return ss.str();
}

std::string dump_fields(Il2CppClass *klass) {
    std::stringstream ss;
    void *iter = nullptr;
    while (auto field = il2cpp_class_get_fields(klass, &iter)) {
        auto type = il2cpp_field_get_type(field);
        auto fClass = il2cpp_class_from_type(type);
        ss << "\t" << il2cpp_class_get_name(fClass)
           << " " << il2cpp_field_get_name(field)
           << "; // 0x" << std::hex
           << il2cpp_field_get_offset(field) << "\n";
    }
    return ss.str();
}

std::string dump_class(Il2CppClass *klass) {
    std::stringstream ss;
    ss << "\n// Namespace: "
       << il2cpp_class_get_namespace(klass) << "\n";
    ss << "class " << il2cpp_class_get_name(klass) << "\n{\n";
    ss << dump_fields(klass);
    ss << dump_methods(klass);
    ss << "}\n";
    return ss.str();
}

// ===================== main dump =====================
void dump_il2cpp(const char *outPath) {
    init_il2cpp();

    auto domain = il2cpp_domain_get();
    size_t count = 0;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &count);

    std::ofstream out(outPath);
    if (!out.is_open()) {
        LOGE("open dump file failed");
        return;
    }

    // ⚠️ UNITY 2022: DÙNG REFLECTION
    auto corlib = il2cpp_get_corlib();
    auto asmClass = il2cpp_class_from_name(
        corlib, "System.Reflection", "Assembly");

    auto load = il2cpp_class_get_method_from_name(asmClass, "Load", 1);
    auto getTypes = il2cpp_class_get_method_from_name(asmClass, "GetTypes", 0);

    typedef void *(*Assembly_Load)(void *, Il2CppString *, void *);
    typedef Il2CppArray *(*Assembly_GetTypes)(void *, void *);

    for (int i = 0; i < count; i++) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        auto name = il2cpp_image_get_name(image);
        out << "\n// Dll: " << name << "\n";

        auto dot = std::string(name).rfind('.');
        auto shortName = std::string(name).substr(0, dot);
        auto str = il2cpp_string_new(shortName.c_str());

        auto asmObj = ((Assembly_Load)load->methodPointer)(nullptr, str, nullptr);
        auto types = ((Assembly_GetTypes)getTypes->methodPointer)(asmObj, nullptr);

        for (int j = 0; j < types->max_length; j++) {
            auto klass = il2cpp_class_from_system_type(
                (Il2CppReflectionType *)types->vector[j]);
            out << dump_class(klass);
        }
    }

    out.close();
    LOGI("dump done -> %s", outPath);
}2020/7/4. *)types->vector[j]);
            out << dump_class(klass);
        }
    }

    out.close();
    LOGI("dump done -> %s", outPath);
}                    outPut << "[Out] ";
                }
            }
            auto parameter_class = il2cpp_class_from_type(param);
            outPut << il2cpp_class_get_name(parameter_class) << " "
                   << il2cpp_method_get_param_name(method, i);
            outPut << ", ";
        }
        if (param_count > 0) {
            outPut.seekp(-2, outPut.cur);
        }
        outPut << ") { }\n";
        //TODO GenericInstMethod
    }
    return outPut.str();
}

std::string dump_property(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Properties\n";
    void *iter = nullptr;
    while (auto prop_const = il2cpp_class_get_properties(klass, &iter)) {
        //TODO attribute
        auto prop = const_cast<PropertyInfo *>(prop_const);
        auto get = il2cpp_property_get_get_method(prop);
        auto set = il2cpp_property_get_set_method(prop);
        auto prop_name = il2cpp_property_get_name(prop);
        outPut << "\t";
        Il2CppClass *prop_class = nullptr;
        uint32_t iflags = 0;
        if (get) {
            outPut << get_method_modifier(il2cpp_method_get_flags(get, &iflags));
            prop_class = il2cpp_class_from_type(il2cpp_method_get_return_type(get));
        } else if (set) {
            outPut << get_method_modifier(il2cpp_method_get_flags(set, &iflags));
            auto param = il2cpp_method_get_param(set, 0);
            prop_class = il2cpp_class_from_type(param);
        }
        if (prop_class) {
            outPut << il2cpp_class_get_name(prop_class) << " " << prop_name << " { ";
            if (get) {
                outPut << "get; ";
            }
            if (set) {
                outPut << "set; ";
            }
            outPut << "}\n";
        } else {
            if (prop_name) {
                outPut << " // unknown property " << prop_name;
            }
        }
    }
    return outPut.str();
}

std::string dump_field(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Fields\n";
    auto is_enum = il2cpp_class_is_enum(klass);
    void *iter = nullptr;
    while (auto field = il2cpp_class_get_fields(klass, &iter)) {
        //TODO attribute
        outPut << "\t";
        auto attrs = il2cpp_field_get_flags(field);
        auto access = attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;
        switch (access) {
            case FIELD_ATTRIBUTE_PRIVATE:
                outPut << "private ";
                break;
            case FIELD_ATTRIBUTE_PUBLIC:
                outPut << "public ";
                break;
            case FIELD_ATTRIBUTE_FAMILY:
                outPut << "protected ";
                break;
            case FIELD_ATTRIBUTE_ASSEMBLY:
            case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
                outPut << "internal ";
                break;
            case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
                outPut << "protected internal ";
                break;
        }
        if (attrs & FIELD_ATTRIBUTE_LITERAL) {
            outPut << "const ";
        } else {
            if (attrs & FIELD_ATTRIBUTE_STATIC) {
                outPut << "static ";
            }
            if (attrs & FIELD_ATTRIBUTE_INIT_ONLY) {
                outPut << "readonly ";
            }
        }
        auto field_type = il2cpp_field_get_type(field);
        auto field_class = il2cpp_class_from_type(field_type);
        outPut << il2cpp_class_get_name(field_class) << " " << il2cpp_field_get_name(field);
        //TODO 获取构造函数初始化后的字段值
        if (attrs & FIELD_ATTRIBUTE_LITERAL && is_enum) {
            uint64_t val = 0;
            il2cpp_field_static_get_value(field, &val);
            outPut << " = " << std::dec << val;
        }
        outPut << "; // 0x" << std::hex << il2cpp_field_get_offset(field) << "\n";
    }
    return outPut.str();
}

std::string dump_type(const Il2CppType *type) {
    std::stringstream outPut;
    auto *klass = il2cpp_class_from_type(type);
    outPut << "\n// Namespace: " << il2cpp_class_get_namespace(klass) << "\n";
    auto flags = il2cpp_class_get_flags(klass);
    if (flags & TYPE_ATTRIBUTE_SERIALIZABLE) {
        outPut << "[Serializable]\n";
    }
    //TODO attribute
    auto is_valuetype = il2cpp_class_is_valuetype(klass);
    auto is_enum = il2cpp_class_is_enum(klass);
    auto visibility = flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;
    switch (visibility) {
        case TYPE_ATTRIBUTE_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_PUBLIC:
            outPut << "public ";
            break;
        case TYPE_ATTRIBUTE_NOT_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
        case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
            outPut << "internal ";
            break;
        case TYPE_ATTRIBUTE_NESTED_PRIVATE:
            outPut << "private ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAMILY:
            outPut << "protected ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
            outPut << "protected internal ";
            break;
    }
    if (flags & TYPE_ATTRIBUTE_ABSTRACT && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "static ";
    } else if (!(flags & TYPE_ATTRIBUTE_INTERFACE) && flags & TYPE_ATTRIBUTE_ABSTRACT) {
        outPut << "abstract ";
    } else if (!is_valuetype && !is_enum && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "sealed ";
    }
    if (flags & TYPE_ATTRIBUTE_INTERFACE) {
        outPut << "interface ";
    } else if (is_enum) {
        outPut << "enum ";
    } else if (is_valuetype) {
        outPut << "struct ";
    } else {
        outPut << "class ";
    }
    outPut << il2cpp_class_get_name(klass); //TODO genericContainerIndex
    std::vector<std::string> extends;
    auto parent = il2cpp_class_get_parent(klass);
    if (!is_valuetype && !is_enum && parent) {
        auto parent_type = il2cpp_class_get_type(parent);
        if (parent_type->type != IL2CPP_TYPE_OBJECT) {
            extends.emplace_back(il2cpp_class_get_name(parent));
        }
    }
    void *iter = nullptr;
    while (auto itf = il2cpp_class_get_interfaces(klass, &iter)) {
        extends.emplace_back(il2cpp_class_get_name(itf));
    }
    if (!extends.empty()) {
        outPut << " : " << extends[0];
        for (int i = 1; i < extends.size(); ++i) {
            outPut << ", " << extends[i];
        }
    }
    outPut << "\n{";
    outPut << dump_field(klass);
    outPut << dump_property(klass);
    outPut << dump_method(klass);
    //TODO EventInfo
    outPut << "}\n";
    return outPut.str();
}

void il2cpp_api_init(void *handle) {
    LOGI("il2cpp_handle: %p", handle);
    init_il2cpp_api(handle);
    if (il2cpp_domain_get_assemblies) {
        Dl_info dlInfo;
        if (dladdr((void *) il2cpp_domain_get_assemblies, &dlInfo)) {
            il2cpp_base = reinterpret_cast<uint64_t>(dlInfo.dli_fbase);
        }
        LOGI("il2cpp_base: %" PRIx64"", il2cpp_base);
    } else {
        LOGE("Failed to initialize il2cpp api.");
        return;
    }
    while (!il2cpp_is_vm_thread(nullptr)) {
        LOGI("Waiting for il2cpp_init...");
        sleep(1);
    }
    auto domain = il2cpp_domain_get();
    il2cpp_thread_attach(domain);
}

void il2cpp_dump(const char *outDir) {
    LOGI("dumping...");
    size_t size;
    auto domain = il2cpp_domain_get();
    auto assemblies = il2cpp_domain_get_assemblies(domain, &size);
    std::stringstream imageOutput;
    for (int i = 0; i < size; ++i) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        imageOutput << "// Image " << i << ": " << il2cpp_image_get_name(image) << "\n";
    }
    std::vector<std::string> outPuts;
    if (il2cpp_image_get_class) {
        LOGI("Version greater than 2018.3");
        //使用il2cpp_image_get_class
        for (int i = 0; i < size; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            std::stringstream imageStr;
            imageStr << "\n// Dll : " << il2cpp_image_get_name(image);
            auto classCount = il2cpp_image_get_class_count(image);
            for (int j = 0; j < classCount; ++j) {
                auto klass = il2cpp_image_get_class(image, j);
                auto type = il2cpp_class_get_type(const_cast<Il2CppClass *>(klass));
                //LOGD("type name : %s", il2cpp_type_get_name(type));
                auto outPut = imageStr.str() + dump_type(type);
                outPuts.push_back(outPut);
            }
        }
    } else {
        LOGI("Version less than 2018.3");
        //使用反射
        auto corlib = il2cpp_get_corlib();
        auto assemblyClass = il2cpp_class_from_name(corlib, "System.Reflection", "Assembly");
        auto assemblyLoad = il2cpp_class_get_method_from_name(assemblyClass, "Load", 1);
        auto assemblyGetTypes = il2cpp_class_get_method_from_name(assemblyClass, "GetTypes", 0);
        if (assemblyLoad && assemblyLoad->methodPointer) {
            LOGI("Assembly::Load: %p", assemblyLoad->methodPointer);
        } else {
            LOGI("miss Assembly::Load");
            return;
        }
        if (assemblyGetTypes && assemblyGetTypes->methodPointer) {
            LOGI("Assembly::GetTypes: %p", assemblyGetTypes->methodPointer);
        } else {
            LOGI("miss Assembly::GetTypes");
            return;
        }
        typedef void *(*Assembly_Load_ftn)(void *, Il2CppString *, void *);
        typedef Il2CppArray *(*Assembly_GetTypes_ftn)(void *, void *);
        for (int i = 0; i < size; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            std::stringstream imageStr;
            auto image_name = il2cpp_image_get_name(image);
            imageStr << "\n// Dll : " << image_name;
            //LOGD("image name : %s", image->name);
            auto imageName = std::string(image_name);
            auto pos = imageName.rfind('.');
            auto imageNameNoExt = imageName.substr(0, pos);
            auto assemblyFileName = il2cpp_string_new(imageNameNoExt.data());
            auto reflectionAssembly = ((Assembly_Load_ftn) assemblyLoad->methodPointer)(nullptr,
                                                                                        assemblyFileName,
                                                                                        nullptr);
            auto reflectionTypes = ((Assembly_GetTypes_ftn) assemblyGetTypes->methodPointer)(
                    reflectionAssembly, nullptr);
            auto items = reflectionTypes->vector;
            for (int j = 0; j < reflectionTypes->max_length; ++j) {
                auto klass = il2cpp_class_from_system_type((Il2CppReflectionType *) items[j]);
                auto type = il2cpp_class_get_type(klass);
                //LOGD("type name : %s", il2cpp_type_get_name(type));
                auto outPut = imageStr.str() + dump_type(type);
                outPuts.push_back(outPut);
            }
        }
    }
    LOGI("write dump file");
    auto outPath = std::string(outDir).append("/files/dump.cs");
    std::ofstream outStream(outPath);
    outStream << imageOutput.str();
    auto count = outPuts.size();
    for (int i = 0; i < count; ++i) {
        outStream << outPuts[i];
    }
    outStream.close();
    LOGI("dump done!");
}
