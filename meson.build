project('psplit', 'cpp',
    default_options: ['cpp_std=c++17'],
    license: 'MIT',
    version: '0.0.1')

psplit_dep = declare_dependency(include_directories: '.')
e = executable('psplit_test', 'psplit_test.cpp')
test('psplit', e)
