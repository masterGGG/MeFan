<?php
/**
 * Auto generated from notificationSys.proto at 2019-07-08 09:54:40
 *
 * mifan package
 */

namespace Mifan {
/**
 * noteArticle message
 */
class noteArticle extends \ProtobufMessage
{
    /* Field index constants */
    const USERID = 1;
    const ARTICLEID = 2;
    const FEEDID = 3;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::USERID => array(
            'name' => 'userid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::ARTICLEID => array(
            'name' => 'articleid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::FEEDID => array(
            'name' => 'feedid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_STRING,
        ),
    );

    /**
     * Constructs new message container and clears its internal state
     */
    public function __construct()
    {
        $this->reset();
    }

    /**
     * Clears message values and sets default ones
     *
     * @return null
     */
    public function reset()
    {
        $this->values[self::USERID] = null;
        $this->values[self::ARTICLEID] = null;
        $this->values[self::FEEDID] = null;
    }

    /**
     * Returns field descriptors
     *
     * @return array
     */
    public function fields()
    {
        return self::$fields;
    }

    /**
     * Sets value of 'userid' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setUserid($value)
    {
        return $this->set(self::USERID, $value);
    }

    /**
     * Returns value of 'userid' property
     *
     * @return integer
     */
    public function getUserid()
    {
        $value = $this->get(self::USERID);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'userid' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasUserid()
    {
        return $this->get(self::USERID) !== null;
    }

    /**
     * Sets value of 'articleid' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setArticleid($value)
    {
        return $this->set(self::ARTICLEID, $value);
    }

    /**
     * Returns value of 'articleid' property
     *
     * @return integer
     */
    public function getArticleid()
    {
        $value = $this->get(self::ARTICLEID);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'articleid' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasArticleid()
    {
        return $this->get(self::ARTICLEID) !== null;
    }

    /**
     * Sets value of 'feedid' property
     *
     * @param string $value Property value
     *
     * @return null
     */
    public function setFeedid($value)
    {
        return $this->set(self::FEEDID, $value);
    }

    /**
     * Returns value of 'feedid' property
     *
     * @return string
     */
    public function getFeedid()
    {
        $value = $this->get(self::FEEDID);
        return $value === null ? (string)$value : $value;
    }

    /**
     * Returns true if 'feedid' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasFeedid()
    {
        return $this->get(self::FEEDID) !== null;
    }
}
}