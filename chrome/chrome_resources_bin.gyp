# only support dll bin reuse now.
{
  'variables': {
    'chromium_code': 1,
    'bin_path': '../build/$(ConfigurationName)',
    'lib_path': '<(bin_path)/lib',
    'local_path': '<(bin_path)/locales',
    'dest_lib_path': '$(OutDir)/lib',
    'dest_local_path': '$(OutDir)/locales',
  },
  
  'includes': [
  ],
  
  'targets': [
    {
      'target_name': 'packed_resources_bin',
      'type': 'none',
      
      'dependencies': [
      ],
 
      'variables': {
        
      },
      
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(bin_path)/chrome_100_percent.pak',
          ],
        },
        {
          'destination': '<(dest_local_path)',
          'files': [
            '<(local_path)/en-US.pak',
            '<(local_path)/zh-CN.pak',
          ],
        },
      ],
 

    },
  ],
}