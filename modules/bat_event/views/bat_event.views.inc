<?php

/**
 * @file
 * Providing extra functionality for the Unit UI via views.
 */

/**
 * Implements hook_views_data_alter().
 */
function bat_event_views_data_alter(&$data) {
  $data['bat_events']['link_event'] = array(
    'field' => array(
      'title' => t('Link'),
      'help' => t('Provide a link to the event.'),
      'handler' => 'bat_event_handler_link_field',
    ),
  );
  $data['bat_events']['edit_event'] = array(
    'field' => array(
      'title' => t('Edit Link'),
      'help' => t('Provide a link to the edit form for the event.'),
      'handler' => 'bat_event_handler_edit_link_field',
    ),
  );
  $data['bat_events']['delete_event'] = array(
    'field' => array(
      'title' => t('Delete Link'),
      'help' => t('Provide a link to delete the event.'),
      'handler' => 'bat_event_handler_delete_link_field',
    ),
  );
  // This content of this field are decided based on the menu structure that
  // follows bat/event/%event_id/op.
  $data['bat_events']['operations'] = array(
    'field' => array(
      'title' => t('Operations links'),
      'help' => t('Display all operations available for this event.'),
      'handler' => 'bat_event_handler_event_operations_field',
    ),
  );
  $data['bat_events']['start_date'] = array(
    'title' => t('Start Date'),
    'help' => t("A event's arrival date."),
    'field' => array(
      'float' => TRUE,
      'handler' => 'bat_event_handler_date_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'bat_event_handler_date_filter',
    ),
  );
  $data['bat_events']['end_date'] = array(
    'title' => t('End Date'),
    'help' => t("A event's departure date."),
    'field' => array(
      'float' => TRUE,
      'handler' => 'bat_event_handler_date_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'bat_event_handler_date_filter',
    ),
  );
  $data['bat_events']['type']['field'] = array(
    'title' => t('Event Type'),
    'help' => t('The event type label.'),
    'handler' => 'bat_event_handler_event_type_field',
  );

  // Expose the uid as a relationship to users.
  $data['bat_events']['uid'] = array(
    'title' => t('Uid'),
    'help' => t("The owner's user ID."),
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
      'name field' => 'name',
    ),
    'filter' => array(
      'title' => t('Name'),
      'handler' => 'views_handler_filter_user_name',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'relationship' => array(
      'title' => t('Owner'),
      'help' => t("Relate this event to its owner's user account"),
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'base field' => 'uid',
      'field' => 'uid',
      'label' => t('Event owner'),
    ),
  );

  $data['bat_events']['nights'] = array(
    'title' => t('Nights'),
    'help' => t("Event's number of nights."),
    'field' => array(
      'float' => TRUE,
      'handler' => 'bat_event_handler_nights_field',
      'click sortable' => TRUE,
    ),
  );

}