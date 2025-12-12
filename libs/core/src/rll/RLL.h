#ifndef RLL_HPP_
#define RLL_HPP_

#include <exception>
#include <cstring>
#include <string>
#include <mutex>
#include <functional>

#define RLL_VERSION_MAJOR 1
#define RLL_VERSION_MINOR 0
#define RLL_VERSION_PATCH 0 

#define RLL_VERSION 10000

#ifdef _WIN32 
	#define RLL_PLATFORM_IS_WINDOWS
#else
	#define RLL_PLATFORM_IS_UNIX
#endif

namespace rll {

namespace exception {

class rll_exception : public std::exception {};

}

namespace unix_flags {

enum unix_flag {
    LOAD_LAZY = 0x00001,
    LOAD_NOW = 0x00002,
    LOAD_LOCAL = 0x00000,
    LOAD_GLOBAL = 0x00100,
    LOAD_DEEPBIND = 0x00008,
    LOAD_NODELETE = 0x01000,
    LOAD_NOLOAD = 0x00004
};
}

namespace windows_flags {

enum windows_flag {
    DONT_RESOLVE_REFERENCES = 0x00000001,
    IGNORE_CODE_AUTHZ_LEVEL = 0x00000010,
    REQUIRE_SIGNED_TARGET = 0x00000080,
    REQUIRE_CURRENT_DIR_SAFE = 0x00002000,
    LOAD_AS_DATAFILE = 0x00000002,
    LOAD_AS_EXCLUSIVE_DATAFILE = 0x00000040,
    LOAD_AS_IMAGE_RESOURCE = 0x00000020,
    SEARCH_APPLICATION_DIR = 0x00000200,
    SEARCH_DEFAULT_DIRS = 0x00001000,
    SEARCH_DLL_LOAD_DIR = 0x00000100,
    SEARCH_SYSTEM32_DIR = 0x00000800,
    SEARCH_USER_DIRS = 0x00000400,
    SEARCH_WITH_ALTERED_PATH = 0x00000008,
};

}

using windows_flag = windows_flags::windows_flag;
using unix_flag = unix_flags::unix_flag;

class loader_flags {
    private:
        unsigned int uflags;
        unsigned int wflags;
    public:
        loader_flags() : uflags(unix_flags::LOAD_LAZY), wflags(0){}
        loader_flags(std::initializer_list<unix_flag> unix_flags, std::initializer_list<windows_flag> windows_flags);

        void add_flag(unix_flag flag);
        void add_flag(windows_flag flag);

        void remove_flag(unix_flag flag);
        void remove_flag(windows_flag flag);

        bool has_flag(unix_flag flag);
        bool has_flag(windows_flag flag);

        unsigned int get_unix_flags();
        unsigned int get_windows_flags();

        void clear_unix_flags();
        void clear_windows_flags();
};


class shared_library {
	private:
		shared_library(const shared_library&);
		shared_library& operator=(const shared_library&);
		std::string lib_path;
		void * lib_handle;
		static std::mutex _mutex;
		void load(const std::string& path, int flags);
	public:
		shared_library();
		virtual ~shared_library();

		void load(const std::string& path){ load(path, loader_flags()); }

		void load(const std::string& path, loader_flags flags);

		void unload();

		bool is_loaded();

		bool has_symbol(const std::string& name){ return get_symbol_fast(name) != nullptr; }

		void * get_symbol(const std::string& name);

        template<typename object_type>
        object_type * get_object_symbol(const std::string& name){
            return static_cast<object_type *>(get_symbol(name));
        }

        template<typename signature>
        std::function<signature> get_function_symbol(const std::string& name){
            return reinterpret_cast<signature *>(get_symbol(name));
        }

        void * get_symbol_fast(const std::string& name) noexcept;

		const std::string& get_path();

		void * get_platform_handle();

		static std::string get_platform_suffix();
};

#define RLL_DEFINE_EXCEPTION_W_METADATA(EXCP_NAME, METADATA_TYPE, METADATA_NAME, WHAT_RETURN) \
    class EXCP_NAME : public rll_exception { \
        public: \
            METADATA_TYPE METADATA_NAME; \
            EXCP_NAME(METADATA_TYPE METADATA_NAME) : METADATA_NAME(METADATA_NAME){} \
            const char* what() const noexcept { \
                WHAT_RETURN \
            } \
    }; \

#define RLL_DEFINE_EXCEPTION(EXCP_NAME, WHAT_RETURN)\
    class EXCP_NAME : public rll_exception { \
        public: \
            const char* what() const noexcept { \
                WHAT_RETURN \
            } \
    }; \

namespace exception {

RLL_DEFINE_EXCEPTION_W_METADATA(symbol_not_found, std::string, symbol_name, return symbol_name.c_str();)

RLL_DEFINE_EXCEPTION_W_METADATA(library_already_loaded, std::string, library_path, return library_path.c_str();)

RLL_DEFINE_EXCEPTION(library_not_loaded, return "A shared_library has not been loaded with content before use.";)

RLL_DEFINE_EXCEPTION_W_METADATA(library_loading_error, std::string, loading_error, return (loading_error != "" ? loading_error.c_str() : "Unknown Error.");)
}

#ifdef RLL_PLATFORM_IS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef ERROR
#include "platform/sl_windows_impl.inl"
#else
#include <dlfcn.h>
#include "platform/sl_unix_impl.inl"
#endif

loader_flags::loader_flags(std::initializer_list<unix_flag> unix_flags, std::initializer_list<windows_flag> windows_flags){
    uflags = 0;
    wflags = 0;
    for(auto& it : unix_flags){
        add_flag(it);
    }
    for(auto& it : windows_flags){
        add_flag(it);
    }
}

void loader_flags::add_flag(unix_flag flag){ 
    if(flag == unix_flags::LOAD_LAZY){
        if(this->has_flag(unix_flags::LOAD_NOW)){
            remove_flag(unix_flags::LOAD_NOW);
        }
    } else if(flag == unix_flags::LOAD_NOW){
        if(this->has_flag(unix_flags::LOAD_LAZY)){
            remove_flag(unix_flags::LOAD_LAZY);
        }
    }

    uflags |= flag; 
}

void loader_flags::add_flag(windows_flag flag){ wflags |= flag; }

void loader_flags::remove_flag(unix_flag flag){ 
    if(flag == unix_flags::LOAD_LAZY){
        uflags &= ~flag;
        add_flag(unix_flags::LOAD_NOW);
    } else if(flag == unix_flags::LOAD_NOW){
        uflags &= ~flag;
        add_flag(unix_flags::LOAD_LAZY);
    }

    uflags &= ~flag; 
}
void loader_flags::remove_flag(windows_flag flag){ wflags &= ~flag; }

bool loader_flags::has_flag(unix_flag flag){
    return true ? ((uflags & flag) == flag) : false;
}
bool loader_flags::has_flag(windows_flag flag){
    return true ? ((wflags & flag) == flag) : false;
}

void loader_flags::clear_unix_flags(){ uflags = unix_flags::LOAD_LAZY; }
void loader_flags::clear_windows_flags(){ wflags = 0; }

unsigned int loader_flags::get_unix_flags(){ return uflags; }
unsigned int loader_flags::get_windows_flags(){ return wflags; }

}

#endif
