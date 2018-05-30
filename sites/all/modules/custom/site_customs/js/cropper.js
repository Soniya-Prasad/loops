(function($) {
  Drupal.behaviors.cropper = {
    attach: function(context, settings) {
      var ValidImage = true;
      // Handle #states
      $('#edit-field-source-und').bind('change',function(){
        if($(this).val() == 'user') {
          $('#edit-field-expert-name-und-0-nid').val('');
          $('#edit-field-student-name-und-0-nid').val('');
          $('#edit-field-loop-staff-name-und-0-nid').val('');
          $('#edit-field-reference-proposed-article-und-0-nid').val('');
          $('#edit-field-by-line-und-0-value').val('');
        } else if($(this).val() == 'expert'){
          $('#edit-field-loop-staff-name-und-0-nid').val('');
          $('#edit-field-student-name-und-0-nid').val('');
          $('#edit-field-user-name-und-0-uid').val('');
          $('#edit-field-reference-proposed-article-und-0-nid').val('');   
          $('#edit-field-by-line-und-0-value').val('');       
        } else if($(this).val() == 'admin') {
          $('#edit-field-expert-name-und-0-nid').val('');
          $('#edit-field-student-name-und-0-nid').val('');
          $('#edit-field-user-name-und-0-uid').val('');
          $('#edit-field-reference-proposed-article-und-0-nid').val('');
          $('#edit-field-by-line-und-0-value').val('');
        } else if($(this).val() == 'custom_byline') {
          $('#edit-field-loop-staff-name-und-0-nid').val('');
          $('#edit-field-expert-name-und-0-nid').val('');
          $('#edit-field-student-name-und-0-nid').val('');
          $('#edit-field-user-name-und-0-uid').val('');
          $('#edit-field-reference-proposed-article-und-0-nid').val('');
        }
      });
      $(':input[name="field_select_user_or_student[und]"]').bind('change',function(){
        if($(this).val() == 'user') {
          $('#edit-field-student-name-und-0-nid').val('');
        } else if($(this).val() == 'student') {
          $('#edit-field-user-name-und-0-uid').val('');
          $('#edit-field-reference-proposed-article-und-0-nid').val('');
        }
      });
      var imageEx = $('#edit-field-article-thumbnail-image-und-0-upload').val();
      if(!imageEx) {
        ValidImage = false;
        var $uploadCrop_square;
        var $uploadCrop_rectangular;
        var $uploadCrop_vertical;
        $('#square_variant').croppie('destroy');
        $('#rectangular_variant').croppie('destroy');
        $('#vertical_variant').croppie('destroy');
        
        $uploadCrop_square = $('#square_variant').croppie({
          viewport: {
            width: 300,
            height: 250,
          },
          boundary: {
            width: 300,
            height: 300
          },
          zoom: 0.8,
          enableExif: false
        });
        
        $uploadCrop_rectangular = $('#rectangular_variant').croppie({
          viewport: {
            width: 320,
            height: 180,
          },
          boundary: {
            width: 400,
            height: 400
          },
          zoom: 0.8,
          enableExif: false
        });
        
        $uploadCrop_vertical = $('#vertical_variant').croppie({
          viewport: {
            width: 320,
            height: 380,
          },
          boundary: {
            width: 400,
            height: 400
          },
          zoom: 0.8,
          enableExif: false
        });
      }
      function readFile(input) {
        if (input.files && input.files[0]) {
          var reader = new FileReader();
          reader.onload = function (e) {
            ValidImage = true;
            $uploadCrop_square.croppie('bind', {
              url: e.target.result,
              zoom: 0.8,
            }).then(function(){
              console.log('jQuery bind complete');
            });
            
            $uploadCrop_rectangular.croppie('bind', {
              url: e.target.result,
              zoom: 0.8,
            }).then(function(){
              console.log('jQuery bind complete');
            });
            
            $uploadCrop_vertical.croppie('bind', {
              url: e.target.result,
              zoom: 0.8,
            }).then(function(){
              console.log('jQuery bind complete');
            });
            $('#variant_container').show();
          }
          reader.readAsDataURL(input.files[0]);
        }else {
          alert("Sorry - you're browser doesn't support the FileReader API");
        }
      }
      
      $('#edit-field-article-thumbnail-image-und-0-upload').on('change', function () {
        var file = this.files[0];
        if(file){
          var ext = file.name.split('.').pop().toLowerCase();;
          if($.inArray(ext, ['gif','png','jpg','jpeg']) == -1) {
            alert('Please upload a valid Image');
          }else{
            readFile(this);
          }
        } 
      });
      
      var im = $('.form-item-field-article-thumbnail-image-und-0 .file a').attr('href');
      var setting_im = false;
      if(settings.hasOwnProperty('croppie')) {
        if(settings.croppie.thumbnail_image){
          if(im != settings.croppie.thumbnail_image) {
            setting_im = true;
          }
        } else {
          setting_im = true;
        }
      } else {
        setting_im = true;
      }
      if(!im) {
        im = imageEx;
      }  
      console.log(im + " - " + setting_im);    
      if(im && setting_im) {
        
          $uploadCrop_square.croppie('bind', {
            url: im,
            zoom: 0.8
          }).then(function(){
            console.log('jQuery bind complete');
          });
          $uploadCrop_rectangular.croppie('bind', {
            url: im,
            zoom: 0.8
          }).then(function(){
            console.log('jQuery bind complete');
          });
          
          $uploadCrop_vertical.croppie('bind', {
            url: im,
            zoom: 0.8
          }).then(function(){
            console.log('jQuery bind complete');
          });
          $('#variant_container').show();
      } else {
        $('#variant_container').hide();
      }
      
      
      
      $('#edit-field-article-thumbnail-image-und-0-remove-button').on('click',function(){
        $('#square_variant').croppie('destroy');
        $('#rectangular_variant').croppie('destroy');
        $('#vertical_variant').croppie('destroy');
        $('#variant_container').hide();
      });
      
      
      if(settings.hasOwnProperty('croppie')) {
        if(settings.croppie.thumbnail_image){
          $uploadCrop_square.croppie('bind', {
            url: settings.croppie.thumbnail_image,
            zoom: 0.8,
            points: (settings.croppie.square_points)?JSON.parse(settings.croppie.square_points):'',
          }).then(function(){
            console.log('jQuery bind complete');
          });
          
          $uploadCrop_rectangular.croppie('bind', {
            url: settings.croppie.thumbnail_image,
            zoom: 0.8,
            points: (settings.croppie.rectangle_points)?JSON.parse(settings.croppie.rectangle_points):'',
          }).then(function(){
            console.log('jQuery bind complete');
          });
          
          $uploadCrop_vertical.croppie('bind', {
            url: settings.croppie.thumbnail_image,
            zoom: 0.8,
            points: (settings.croppie.rectangle_points)?JSON.parse(settings.croppie.vertical_points):'',
          }).then(function(){
            console.log('jQuery bind complete');
          });
          $('#variant_container').show();
        }
      }
      
      $uploadCrop_square.on('update', function(event) {
        ValidImage = true;
      });
      $uploadCrop_rectangular.on('update', function(event) {
        ValidImage = true;
      });
      $uploadCrop_vertical.on('update', function(event) {
        ValidImage = true;
      });
      
      
      $('.node-article-form').on('submit', function (ev) {
        if(ValidImage){
          $uploadCrop_square.croppie('result', {
            type: 'canvas',
            size: {width: 760, height: 600},
            format: 'jpeg'
          }).then(function (resp) {
            $('#edit-field-square-variant-image-data-und-0-value').val('');
            $('#edit-field-square-variant-points-und-0-value').val('');
            $('#edit-field-square-variant-points-und-0-value').val($uploadCrop_square.croppie('get').points);
            $('#edit-field-square-variant-image-data-und-0-value').val(resp);
          });
          
          $uploadCrop_rectangular.croppie('result', {
            type: 'canvas',
            size: {width: 870, height: 470},
            format: 'jpeg' 
          }).then(function (resp) {
            $('#edit-field-rectangular-variant-points-und-0-value').val('');
            $('#edit-field-rect-variant-image-data-und-0-value').val('');
            $('#edit-field-rectangular-variant-points-und-0-value').val($uploadCrop_rectangular.croppie('get').points);
            $('#edit-field-rect-variant-image-data-und-0-value').val(resp);
          });
          
          $uploadCrop_vertical.croppie('result', {
            type: 'canvas',
            size: {width: 320, height: 380},
            format: 'jpeg'
          }).then(function (resp) {
            $('#edit-field-vertical-variant-points-und-0-value').val('');
            $('#edit-field-vert-variant-image-data-und-0-value').val('');
            $('#edit-field-vertical-variant-points-und-0-value').val($uploadCrop_vertical.croppie('get').points);
            $('#edit-field-vert-variant-image-data-und-0-value').val(resp);
          });
        }
      });
    }
  }
})(jQuery);
