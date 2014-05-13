# only support dll bin reuse now.
{
  'variables': {
    'chromium_code': 1,
    'bin_path': '../build/$(ConfigurationName)',
    'lib_path': '<(bin_path)/lib',
    'dest_lib_path': '$(OutDir)/lib/',
  },
  
  'includes': [
  ],
  
  'targets': [
    {
      'target_name': 'skia_bin',
      'type': 'none',
      
      # These two set the paths so we can include skia/gyp/core.gypi
      'skia_src_path': '../third_party/skia/src',
      'skia_include_path': '../third_party/skia/include',
      

      # just source list
      'includes': [
      #  '../third_party/skia/gyp/core.gypi',
      #  '../third_party/skia/gyp/effects.gypi',
      ],
      
      'dependencies': [
      ],
 
      'direct_dependent_settings': {
        'include_dirs': [
          'config',

          #temporary until we can hide SkFontHost
          '../third_party/skia/src/core',

          '../third_party/skia/include/config',
          '../third_party/skia/include/core',
          '../third_party/skia/include/effects',
          '../third_party/skia/include/pdf',
          '../third_party/skia/include/gpu',
          '../third_party/skia/include/gpu/gl',
          '../third_party/skia/include/pathops',
          '../third_party/skia/include/pipe',
          '../third_party/skia/include/ports',
          '../third_party/skia/include/utils',
          'ext',
        ],
        
        'defines': [
          'SK_BUILD_NO_IMAGE_ENCODE',
          'SK_DEFERRED_CANVAS_USES_GPIPE=1',
          'GR_GL_CUSTOM_SETUP_HEADER="GrGLConfig_chrome.h"',
          'GR_AGGRESSIVE_SHADER_OPTS=1',
          'SK_ENABLE_INST_COUNT=0',
          'GR_DLL',
          'SKIA_DLL',
           
        ],
        
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'skia.lib',
            ],
            'AdditionalLibraryDirectories': [
              '<(dest_lib_path)'
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(bin_path)/skia.dll',
            '<(bin_path)/skia.dll.pdb',
          ],
        },
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)/skia.lib'
          ],
        },
      ],  
      
      'sources': [
        # this should likely be moved into src/utils in skia
        '../third_party/skia/src/core/SkFlate.cpp',
        # We don't want to add this to Skia's core.gypi since it is
        # Android only. Include it here and remove it for everyone
        # but Android later.
        '../third_party/skia/src/core/SkPaintOptionsAndroid.cpp',

        #'../third_party/skia/src/images/bmpdecoderhelper.cpp',
        #'../third_party/skia/src/images/bmpdecoderhelper.h',
        #'../third_party/skia/src/images/SkFDStream.cpp',
        #'../third_party/skia/src/images/SkImageDecoder.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_FactoryDefault.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_FactoryRegistrar.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_fpdfemb.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_libbmp.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_libgif.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_libico.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_libjpeg.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_libpng.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_libpvjpeg.cpp',
        #'../third_party/skia/src/images/SkImageDecoder_wbmp.cpp',
        #'../third_party/skia/src/images/SkImageEncoder.cpp',
        #'../third_party/skia/src/images/SkImageEncoder_Factory.cpp',
        #'../third_party/skia/src/images/SkImageRef.cpp',
        #'../third_party/skia/src/images/SkImageRefPool.cpp',
        #'../third_party/skia/src/images/SkImageRefPool.h',
        #'../third_party/skia/src/images/SkImageRef_GlobalPool.cpp',
        #'../third_party/skia/src/images/SkMovie.cpp',
        #'../third_party/skia/src/images/SkMovie_gif.cpp',
        '../third_party/skia/src/images/SkScaledBitmapSampler.cpp',
        '../third_party/skia/src/images/SkScaledBitmapSampler.h',

        '../third_party/skia/src/opts/opts_check_SSE2.cpp',

        '../third_party/skia/src/pdf/SkPDFCatalog.cpp',
        '../third_party/skia/src/pdf/SkPDFCatalog.h',
        '../third_party/skia/src/pdf/SkPDFDevice.cpp',
        '../third_party/skia/src/pdf/SkPDFDocument.cpp',
        '../third_party/skia/src/pdf/SkPDFFont.cpp',
        '../third_party/skia/src/pdf/SkPDFFont.h',
        '../third_party/skia/src/pdf/SkPDFFormXObject.cpp',
        '../third_party/skia/src/pdf/SkPDFFormXObject.h',
        '../third_party/skia/src/pdf/SkPDFGraphicState.cpp',
        '../third_party/skia/src/pdf/SkPDFGraphicState.h',
        '../third_party/skia/src/pdf/SkPDFImage.cpp',
        '../third_party/skia/src/pdf/SkPDFImage.h',
        '../third_party/skia/src/pdf/SkPDFImageStream.cpp',
        '../third_party/skia/src/pdf/SkPDFImageStream.h',
        '../third_party/skia/src/pdf/SkPDFPage.cpp',
        '../third_party/skia/src/pdf/SkPDFPage.h',
        '../third_party/skia/src/pdf/SkPDFShader.cpp',
        '../third_party/skia/src/pdf/SkPDFShader.h',
        '../third_party/skia/src/pdf/SkPDFStream.cpp',
        '../third_party/skia/src/pdf/SkPDFStream.h',
        '../third_party/skia/src/pdf/SkPDFTypes.cpp',
        '../third_party/skia/src/pdf/SkPDFTypes.h',
        '../third_party/skia/src/pdf/SkPDFUtils.cpp',
        '../third_party/skia/src/pdf/SkPDFUtils.h',

        #'../third_party/skia/src/ports/SkPurgeableMemoryBlock_android.cpp',
        #'../third_party/skia/src/ports/SkPurgeableMemoryBlock_mac.cpp',
        '../third_party/skia/src/ports/SkPurgeableMemoryBlock_none.cpp',

        '../third_party/skia/src/ports/SkFontConfigInterface_android.cpp',
        #'../third_party/skia/src/ports/SkFontHost_FONTPATH.cpp',
        '../third_party/skia/src/ports/SkFontHost_FreeType.cpp',
        '../third_party/skia/src/ports/SkFontHost_FreeType_common.cpp',
        '../third_party/skia/src/ports/SkFontHost_FreeType_common.h',
        '../third_party/skia/src/ports/SkFontConfigParser_android.cpp',
        #'../third_party/skia/src/ports/SkFontHost_ascender.cpp',
        #'../third_party/skia/src/ports/SkFontHost_linux.cpp',
        '../third_party/skia/src/ports/SkFontHost_mac.cpp',
        #'../third_party/skia/src/ports/SkFontHost_none.cpp',
        '../third_party/skia/src/ports/SkFontHost_win.cpp',
        '../third_party/skia/src/ports/SkGlobalInitialization_chromium.cpp',
        #'../third_party/skia/src/ports/SkImageDecoder_CG.cpp',
        #'../third_party/skia/src/ports/SkImageDecoder_empty.cpp',
        #'../third_party/skia/src/ports/SkImageRef_ashmem.cpp',
        #'../third_party/skia/src/ports/SkImageRef_ashmem.h',
        #'../third_party/skia/src/ports/SkOSEvent_android.cpp',
        #'../third_party/skia/src/ports/SkOSEvent_dummy.cpp',
        '../third_party/skia/src/ports/SkOSFile_stdio.cpp',
        #'../third_party/skia/src/ports/SkThread_none.cpp',
        '../third_party/skia/src/ports/SkThread_pthread.cpp',
        '../third_party/skia/src/ports/SkThread_win.cpp',
        '../third_party/skia/src/ports/SkTime_Unix.cpp',
        #'../third_party/skia/src/ports/SkXMLParser_empty.cpp',
        #'../third_party/skia/src/ports/SkXMLParser_expat.cpp',
        #'../third_party/skia/src/ports/SkXMLParser_tinyxml.cpp',
        #'../third_party/skia/src/ports/SkXMLPullParser_expat.cpp',

        '../third_party/skia/src/sfnt/SkOTUtils.cpp',
        '../third_party/skia/src/sfnt/SkOTUtils.h',

        '../third_party/skia/include/utils/mac/SkCGUtils.h',
        '../third_party/skia/include/utils/SkDeferredCanvas.h',
        '../third_party/skia/include/utils/SkMatrix44.h',
        '../third_party/skia/src/utils/mac/SkCreateCGImageRef.cpp',
        '../third_party/skia/src/utils/SkBase64.cpp',
        '../third_party/skia/src/utils/SkBase64.h',
        '../third_party/skia/src/utils/SkBitSet.cpp',
        '../third_party/skia/src/utils/SkBitSet.h',
        '../third_party/skia/src/utils/SkDeferredCanvas.cpp',
        '../third_party/skia/src/utils/SkMatrix44.cpp',
        '../third_party/skia/src/utils/SkNullCanvas.cpp',
        '../third_party/skia/include/utils/SkNWayCanvas.h',
        '../third_party/skia/src/utils/SkNWayCanvas.cpp',
        '../third_party/skia/src/utils/SkPictureUtils.cpp',
        '../third_party/skia/src/utils/SkRTConf.cpp',
        '../third_party/skia/include/utils/SkRTConf.h',
        '../third_party/skia/include/pdf/SkPDFDevice.h',
        '../third_party/skia/include/pdf/SkPDFDocument.h',

        '../third_party/skia/include/ports/SkTypeface_win.h',

        #'../third_party/skia/include/images/SkImageDecoder.h',
        #'../third_party/skia/include/images/SkImageEncoder.h',
        '../third_party/skia/include/images/SkImageRef.h',
        '../third_party/skia/include/images/SkImageRef_GlobalPool.h',
        '../third_party/skia/include/images/SkMovie.h',
        '../third_party/skia/include/images/SkPageFlipper.h',

        '../third_party/skia/include/utils/SkNullCanvas.h',
        '../third_party/skia/include/utils/SkPictureUtils.h',
        'ext/analysis_canvas.cc',
        'ext/analysis_canvas.h',
        'ext/bitmap_platform_device.h',
        'ext/bitmap_platform_device_android.cc',
        'ext/bitmap_platform_device_android.h',
        'ext/bitmap_platform_device_data.h',
        'ext/bitmap_platform_device_linux.cc',
        'ext/bitmap_platform_device_linux.h',
        'ext/bitmap_platform_device_mac.cc',
        'ext/bitmap_platform_device_mac.h',
        'ext/bitmap_platform_device_win.cc',
        'ext/bitmap_platform_device_win.h',
        'ext/convolver.cc',
        'ext/convolver.h',
        'ext/google_logging.cc',
        'ext/image_operations.cc',
        'ext/image_operations.h',
        'ext/lazy_pixel_ref.cc',
        'ext/lazy_pixel_ref.h',
        'ext/SkThread_chrome.cc',
        'ext/paint_simplifier.cc',
        'ext/paint_simplifier.h',
        'ext/platform_canvas.cc',
        'ext/platform_canvas.h',
        'ext/platform_device.cc',
        'ext/platform_device.h',
        'ext/platform_device_linux.cc',
        'ext/platform_device_mac.cc',
        'ext/platform_device_win.cc',
        'ext/recursive_gaussian_convolution.cc',
        'ext/recursive_gaussian_convolution.h',
        'ext/refptr.h',
        'ext/SkMemory_new_handler.cpp',
        'ext/skia_trace_shim.h',
        'ext/skia_utils_base.cc',
        'ext/skia_utils_base.h',
        'ext/skia_utils_ios.mm',
        'ext/skia_utils_ios.h',
        'ext/skia_utils_mac.mm',
        'ext/skia_utils_mac.h',
        'ext/skia_utils_win.cc',
        'ext/skia_utils_win.h',
        'ext/vector_canvas.cc',
        'ext/vector_canvas.h',
        'ext/vector_platform_device_emf_win.cc',
        'ext/vector_platform_device_emf_win.h',
        'ext/vector_platform_device_skia.cc',
        'ext/vector_platform_device_skia.h',
      ],
      
    },
  ],
}