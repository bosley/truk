#ifndef RLL_PLATFORM_INCLUDED
#define RLL_PLATFORM_INCLUDED

std::mutex shared_library_c::_mutex;

shared_library_c::shared_library_c(){
	lib_handle = nullptr;
}

shared_library_c::~shared_library_c(){
	unload();
}

void shared_library_c::load(const std::string& path, int flags){
	std::lock_guard<std::mutex> lock(_mutex);

	if(lib_handle != nullptr){ 
		throw exception::library_already_loaded(path);
	}

	lib_handle = dlopen(path.c_str(), flags);
	
	if(lib_handle == nullptr){
		const char* error = dlerror();
		throw exception::library_loading_error(error);
	}
	
	lib_path = path;
}

void shared_library_c::load(const std::string& path, loader_flags flags){
	load(path, flags.get_unix_flags());
}

void shared_library_c::unload(){
	std::lock_guard<std::mutex> lock(_mutex);

	if(lib_handle != nullptr){
		dlclose(lib_handle);
		lib_handle = nullptr;
	}

	lib_path.clear();
}


bool shared_library_c::is_loaded(){
	return lib_handle != nullptr;
}


void * shared_library_c::get_symbol(const std::string& name){
	std::lock_guard<std::mutex> lock(_mutex);

	if(lib_handle != nullptr){
		void * result = dlsym(lib_handle, name.c_str());
		char * error = dlerror();

		if(error != nullptr){
			if(std::strcmp(error, "") != 0){
				throw exception::symbol_not_found(name);
			}
		}
		
		return result;
	} else {
		throw exception::library_not_loaded();
	}
}

void * shared_library_c::get_symbol_fast(const std::string& name) noexcept {
	std::lock_guard<std::mutex> lock(_mutex);

	if(lib_handle != nullptr){
		return dlsym(lib_handle, name.c_str());
	} else {
		return nullptr;
	}
}


const std::string& shared_library_c::get_path(){
	return lib_path;
}

void * shared_library_c::get_platform_handle(){
	return lib_handle;
}

std::string shared_library_c::get_platform_suffix(){
	#if defined(__APPLE__)
		return ".dylib";
	#elif defined(__CYGWIN__)
		return ".dll";
	#else
		return ".so";
	#endif
}

#endif
