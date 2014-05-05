# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/chrome',
    'repack_locales_cmd': ['<(DEPTH)/setup_env.bat&&python', 'tools/build/repack_locales.py'],
  },
  'targets': [
  
    {
      'target_name': 'packed_resources',
      'type': 'none',
      'variables': {
        'repack_path': '../tools/grit/grit/format/repack.py',
      },
      'dependencies': [
        # MSVS needs the dependencies explictly named, Make is able to
        # derive the dependencies from the output files.
        
        '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
      ],
      'actions': [
        {
          'includes': ['chrome_repack_locales.gypi']
        },
        {
          'includes': ['chrome_repack_pseudo_locales.gypi']
        },
        {
          'includes': ['chrome_repack_chrome_100_percent.gypi']
        },
      ],
      'conditions': [

        ['OS != "mac" and OS != "ios"', {
          # Copy pak files to the product directory. These files will be picked
          # up by the following installer scripts:
          #   - Windows: chrome/installer/mini_installer/chrome.release
          #   - Linux: chrome/installer/linux/internal/common/installer.include
          # Ensure that the above scripts are updated when adding or removing
          # pak files.
          # Copying files to the product directory is not needed on the Mac
          # since the framework build phase will copy them into the framework
          # bundle directly.
          'copies': [
            # {
              # 'destination': '<(PRODUCT_DIR)',
              # 'files': [
                # '<(SHARED_INTERMEDIATE_DIR)/repack/chrome.pak'
              # ],
            # },
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                '<(SHARED_INTERMEDIATE_DIR)/repack/chrome_100_percent.pak'
              ],
            },
            {
              'destination': '<(PRODUCT_DIR)/locales',
              'files': [
                '<!@pymod_do_main(repack_locales -o -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(locales))'
              ],
            },
            {
              'destination': '<(PRODUCT_DIR)/pseudo_locales',
              'files': [
                '<!@pymod_do_main(repack_locales -o -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(pseudo_locales))'
              ],
            },
          ],
        }], # end OS != "mac" and OS != "ios"
      ], # conditions
    },

  ], # targets
}
