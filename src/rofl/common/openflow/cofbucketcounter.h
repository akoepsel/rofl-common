/*
 * cofbucketcounter.h
 *
 *  Created on: 16.03.2014
 *      Author: andreas
 */

#ifndef COFBUCKETCOUNTER_H_
#define COFBUCKETCOUNTER_H_

#include <inttypes.h>

#include <iostream>

#include "rofl/common/openflow/openflow.h"
#include "rofl/common/openflow/openflow_rofl_exceptions.h"
#include "rofl/common/exception.hpp"


namespace rofl {
namespace openflow {

class eBucketCounterBase			: public exception {
public:
	eBucketCounterBase(
			const std::string& __arg = std::string("")) :
				exception(__arg)
	{
		set_exception("eBucketCounterBase");
	};
};
class eBucketCounterInval			: public eBucketCounterBase {
public:
	eBucketCounterInval(
			const std::string& __arg = std::string("")) :
				eBucketCounterBase(__arg)
	{
		set_exception("eBucketCounterInval");
	};
};
class eBucketCounterNotFound		: public eBucketCounterBase {
public:
	eBucketCounterNotFound(
			const std::string& __arg = std::string("")) :
				eBucketCounterBase(__arg)
	{
		set_exception("eBucketCounterNotFound");
	};
};

class cofbucket_counter
{
	uint8_t		ofp_version;
	uint64_t	packet_count;
	uint64_t	byte_count;

public:

	/**
	 *
	 */
	cofbucket_counter(
			uint8_t ofp_version = rofl::openflow::OFP_VERSION_UNKNOWN);

	/**
	 *
	 */
	virtual
	~cofbucket_counter();

	/**
	 *
	 */
	cofbucket_counter(
			cofbucket_counter const& bc);

	/**
	 *
	 */
	cofbucket_counter&
	operator= (
			cofbucket_counter const& bc);

	/**
	 *
	 */
	bool
	operator== (
			cofbucket_counter const& bc);

public:

	/**
	 *
	 */
	uint8_t
	get_version() const { return ofp_version; };

	/**
	 *
	 */
	void
	set_version(uint8_t ofp_version) { this->ofp_version = ofp_version; };

	/**
	 *
	 */
	uint64_t
	get_packet_count() const { return packet_count; };

	/**
	 *
	 */
	void
	set_packet_count(uint64_t packet_count) { this->packet_count = packet_count; };

	/**
	 *
	 */
	uint64_t
	get_byte_count() const { return byte_count; };

	/**
	 *
	 */
	void
	set_byte_count(uint64_t byte_count) { this->byte_count = byte_count; };

public:

	/**
	 *
	 */
	virtual size_t
	length() const;

	/**
	 *
	 */
	virtual void
	pack(uint8_t* buf, size_t buflen);

	/**
	 *
	 */
	virtual void
	unpack(uint8_t* buf, size_t buflen);

public:

	friend std::ostream&
	operator<< (std::ostream& os, cofbucket_counter const& bc) {
		os  << "<cofbucket_counter ";
		os << "ofp-version: " << (int)bc.ofp_version << " ";
		os << std::hex;
		os << "packet-count: 0x" << (unsigned long long)bc.packet_count << " ";
		os << "byte-count: 0x" << (unsigned long long)bc.byte_count << " ";
		os << std::dec;
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace openflow
}; // end of namespace rofl

#endif /* COFBUCKETCOUNTER_H_ */
