# only support dll bin reuse now.
{ 
   'variables': {
    'chromium_code': 1,
  },
  
  'targets': [
    {
      'target_name': 'base_simple',
      'type': 'executable',

      'dependencies': [
        '../../base/base_bin.gyp:base_bin'
      ],
      'sources': [
        'main.cc',
      ],
    },
#    {
#    },
  ],
}