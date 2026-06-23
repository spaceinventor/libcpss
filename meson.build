project('cpss', 'c', version: '2.0')

csp_dep = dependency('csp', fallback:['csp', 'csp_dep']).partial_dependency(links: false, includes: true)
rpc_dep = dependency('rpc', fallback:['rpc', 'rpc_dep'])
param_dep = dependency('param', fallback:['param', 'param_dep'])

clib = meson.get_compiler('c').find_library('c', required: false)
clib_dep = []
if not clib.found()
	clib_dep = dependency('libc', fallback: ['picolibc', 'picolibc_dep'], required: true)
endif

lib_deps = [csp_dep, rpc_dep, clib_dep]
lib_deps_client = lib_deps + [param_dep]

lib_src_server = []
gen_src_server = subproject('rpc').get_variable('gen_rpc_server').process('cpss.json5')

include_server = include_directories('include')
cpssserver = library('cpssserver', [lib_src_server, gen_src_server], include_directories: [include_server, include_directories('.')], dependencies: lib_deps)
cpssserver_dep = declare_dependency(include_directories : [include_server, include_directories('.')], link_with : cpssserver, dependencies: lib_deps)

lib_src_client = ['src/cpss_client.c']
gen_src_client = subproject('rpc').get_variable('gen_rpc_client').process('cpss.json5')

include_provider = [include_directories('include'), include_directories('include_provider')]
cpssprovider = library('cpssprovider', [lib_src_client, gen_src_client], include_directories: [include_provider, include_directories('.')], dependencies: lib_deps_client)
cpssprovider_dep = declare_dependency(include_directories : [include_provider, include_directories('.')], link_with : cpssprovider, dependencies: lib_deps_client)

include_consumer = [include_directories('include'), include_directories('include_consumer')]
cpssconsumer = library('cpssconsumer', [lib_src_client, gen_src_client], include_directories: [include_consumer, include_directories('.')], dependencies: lib_deps_client)
cpssconsumer_dep = declare_dependency(include_directories : [include_consumer, include_directories('.')], link_with : cpssconsumer, dependencies: lib_deps_client)
