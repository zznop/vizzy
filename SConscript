Import('env')

libvizzy_env = env.Clone()
libvizzy_env.AppendUnique(
    LIBS=['dl'],
    CPPDEFINES='_GNU_SOURCE',
)
libvizzy = libvizzy_env.SharedLibrary(
    'libvizzy.so',
    source=['src/lib/hooks.c'],
)

incbin_o = env.Object('src/incbin.s')
env.Depends(incbin_o, libvizzy)

sources = [
    'src/main.c',
    'src/log.c',
    incbin_o,
]

vizzy = env.Program(
    'vizzytrace',
    source = sources,
)

Return('vizzy')
