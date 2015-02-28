{
    'targets': [
        {
            'target_name': 'labor',
            'type': 'executable',
            'sources': ['src/labor_connect.cc', 'src/labor_event.cc', 'src/labor_log.cc', 'src/labor_lvm.cc',
                        'src/labor_pvm.cc', 'src/labor_request.cc', 'src/labor_response.cc', 'src/labor_service.cc',
                        'src/labor_utils.cc', 'src/labor_main.cc',
            ],

            'conditions': [
                ['OS=="linux"',
                {
                    'cflags': ['-std=c++11'],
                    'ldflags': ['-lpthread', '-lpython27', '-lzmq', '-llua']
                }],


                ['OS=="win"',
                {
                    'defines': ['_CRT_SECURE_NO_WARNINGS', '_SCL_SECURE_NO_WARNINGS'],
                    'include_dirs': ['./win-deps/include', './win-deps/lib'],
                    'msvc_settings': { 'VCCompilerTool': { 'RuntimeLibrary': 0 } },
                    'defines': ['WIN32'],
                    'configurations':
                    {
                        'Debug': {
                            'defines': ['LABOR_DEBUG', '_DEBUG', 'DEBUG'],
                            'msvs_configuration_platform': 'Win32',
                            'dependence': ['libzmq_d', 'pthread_d', 'python27_d', 'lua_d']
                        },
                        'Debug_X64': {
                            'inherit_from': ['Debug'],
                            'msvs_configuration_platform': 'x64'
                        },
                        'Release': {
                            'msvs_configuration_platform': 'Win32',
                            'dependence': ['libzmq', 'pthread', 'python27', 'lua']
                        },
                        'Release_X64': {
                            'inherit_from': ['Release'],
                            'msvs_configuration_platform': 'x64'
                        }
                    }
                }]
            ],
        },
    ]
}
