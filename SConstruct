env = Environment(
    SRCDIR = 'src',
    BUILDROOT = 'build',
    CC = 'gcc',
    CCFLAGS = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-std=c99',
        '-O2',
    ],
    CPPPATH=['lib'],
)

env.SConscript(
    './SConscript',
    variant_dir=env['BUILDROOT'],
    duplicate=False,
    exports='env',
)
