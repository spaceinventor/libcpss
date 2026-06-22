project('cpss', 'c', version: '2.0')

csp_dep = dependency('csp', fallback:['csp', 'csp_dep']).partial_dependency(links: false, includes: true)
rpc_dep = dependency('rpc', fallback:['rpc', 'rpc_dep'])

clib = meson.get_compiler('c').find_library('c', required: false)
clib_dep = []
if not clib.found()
	clib_dep = dependency('libc', fallback: ['picolibc', 'picolibc_dep'], required: true)
endif

lib_deps = [csp_dep, rpc_dep, clib_dep]

lib_src_server = []
gen_src_server = subproject('rpc').get_variable('gen_rpc_server').process('cpss.json5')

include_server = include_directories('include')
cpssserver = library('cpssserver', [lib_src_server, gen_src_server], include_directories: [include_server, include_directories('.')], dependencies: lib_deps)
cpssserver_dep = declare_dependency(include_directories : [include_server, include_directories('.')], link_with : cpssserver, dependencies: lib_deps)

lib_src_client = []
gen_src_client = subproject('rpc').get_variable('gen_rpc_client').process('cpss.json5')

include_consumer = [include_directories('include'), include_directories('include_consumer')]
cpssconsumer = library('cpssconsumer', [lib_src_client, gen_src_client], include_directories: [include_consumer, include_directories('.')], dependencies: lib_deps)
cpssconsumer_dep = declare_dependency(include_directories : [include_consumer, include_directories('.')], link_with : cpssconsumer, dependencies: lib_deps)
