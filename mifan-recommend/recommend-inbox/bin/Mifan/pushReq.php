<?php
/**
 * Auto generated from protoRecommend.proto at 2019-07-23 18:17:52
 *
 * mifan package
 */

namespace Mifan {
/**
 * pushReq message
 */
class pushReq extends \ProtobufMessage
{
    /* Field index constants */
    const USERID = 1;
    const INDEX = 2;
    const INFO = 3;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::USERID => array(
            'name' => 'userid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::INDEX => array(
            'name' => 'index',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::INFO => array(
            'name' => 'info',
            'repeated' => true,
            'type' => '\Mifan\pushReq_feedInfo'
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
        $this->values[self::INDEX] = null;
        $this->values[self::INFO] = array();
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
     * Sets value of 'index' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setIndex($value)
    {
        return $this->set(self::INDEX, $value);
    }

    /**
     * Returns value of 'index' property
     *
     * @return integer
     */
    public function getIndex()
    {
        $value = $this->get(self::INDEX);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'index' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasIndex()
    {
        return $this->get(self::INDEX) !== null;
    }

    /**
     * Appends value to 'info' list
     *
     * @param \Mifan\pushReq_feedInfo $value Value to append
     *
     * @return null
     */
    public function appendInfo(\Mifan\pushReq_feedInfo $value)
    {
        return $this->append(self::INFO, $value);
    }

    /**
     * Clears 'info' list
     *
     * @return null
     */
    public function clearInfo()
    {
        return $this->clear(self::INFO);
    }

    /**
     * Returns 'info' list
     *
     * @return \Mifan\pushReq_feedInfo[]
     */
    public function getInfo()
    {
        return $this->get(self::INFO);
    }

    /**
     * Returns true if 'info' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasInfo()
    {
        return count($this->get(self::INFO)) !== 0;
    }

    /**
     * Returns 'info' iterator
     *
     * @return \ArrayIterator
     */
    public function getInfoIterator()
    {
        return new \ArrayIterator($this->get(self::INFO));
    }

    /**
     * Returns element from 'info' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return \Mifan\pushReq_feedInfo
     */
    public function getInfoAt($offset)
    {
        return $this->get(self::INFO, $offset);
    }

    /**
     * Returns count of 'info' list
     *
     * @return int
     */
    public function getInfoCount()
    {
        return $this->count(self::INFO);
    }
}
}