<?php

/**
 * @file
 * Interface EventStyle
 */

namespace Roomify\bat_event;

interface EventStyle {

  /**
   * @param $event_type
   */
  public function format($event_type);

}
