(function($) {
  Drupal.behaviors.event = {
    attach: function(context, settings) {
      $(".field-name-field-show-time input:checkbox").on('change',function() {
        if(!$(this).is(':checked')) {
          $(".field-name-field-date .form-item-field-date-und-0-value-time").hide();
          $(".field-name-field-date .form-item-field-date-und-0-value-time input").val('12:00am');
          $(".field-name-field-date .form-item-field-date-und-0-value2-time").hide();
          $(".field-name-field-date .form-item-field-date-und-0-value2-time input").val('11:59pm');
        }
        else {
          $(".field-name-field-date .form-item-field-date-und-0-value-time").show();
          $(".field-name-field-date .form-item-field-date-und-0-value-time input").val('');
          $(".field-name-field-date .form-item-field-date-und-0-value2-time").show();
          $(".field-name-field-date .form-item-field-date-und-0-value2-time input").val('');
        }
      });

      if(!$(".field-name-field-show-time input:checkbox").is(':checked')) {
        $(".field-name-field-date .form-item-field-date-und-0-value-time").hide();
        $(".field-name-field-date .form-item-field-date-und-0-value2-time").hide();
      }
    }
  }
})(jQuery);
