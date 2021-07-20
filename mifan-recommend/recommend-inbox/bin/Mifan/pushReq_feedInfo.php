<?php
/**
 * Auto generated from protoRecommend.proto at 2019-07-23 18:17:52
 *
 * mifan package
 */

namespace Mifan {
/**
 * feedInfo message embedded in pushReq message
 */
class pushReq_feedInfo extends \ProtobufMessage
{
    /* Field index constants */
    const FEEDID = 1;
    const SCORE = 2;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::FEEDID => array(
            'name' => 'feedid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_STRING,
        ),
        self::SCORE => array(
            'name' => 'score',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
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
        $this->values[self::FEEDID] = null;
        $this->values[self::SCORE] = null;
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

    /**
     * Sets value of 'score' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setScore($value)
    {
        return $this->set(self::SCORE, $value);
    }

    /**
     * Returns value of 'score' property
     *
     * @return integer
     */
    public function getScore()
    {
        $value = $this->get(self::SCORE);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'score' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasScore()
    {
        return $this->get(self::SCORE) !== null;
    }
}
}