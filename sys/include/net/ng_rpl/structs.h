/*
 * Copyright (C) 2013  INRIA.
 * Copyright (C) 2015 Cenk Gündoğan <cnkgndgn@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup net_ng_rpl
 * @{
 *
 * @file
 * @brief       RPL data structs
 *
 * Header file, which defines all structs used by RPL.
 *
 * @author      Eric Engel <eric.engel@fu-berlin.de>
 * @author      Cenk Gündoğan <cnkgndgn@gmail.com>
 */

#ifndef NG_RPL_STRUCTS_H_
#define NG_RPL_STRUCTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "net/ng_ipv6.h"
#include "trickle.h"

/**
 * @brief RPL-Option Generic Format
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.7.1">
 *          RPL Control Message Option Generic Format
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;       /**< Option Type */
    uint8_t length;     /**< Option Length, does not include the first two byte */
} ng_rpl_opt_t;

/**
 * @brief DIO Base Object
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.3.1">
 *          Format of the DIO Base Object
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t instance_id;        /**< id of the instance */
    uint8_t version_number;     /**< version number of the DODAG */
    network_uint16_t rank;      /**< rank of the parent emitting the DIO */
    uint8_t g_mop_prf;          /**< grounded, MOP, preferred flags */
    uint8_t dtsn;               /**< Destination Advertisement Trigger Sequence Number */
    uint8_t flags;              /**< unused */
    uint8_t reserved;           /**< reserved */
    ipv6_addr_t dodag_id;       /**< id of the dodag */
} ng_rpl_dio_t;

/**
 * @brief DODAG Configuration Option
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.7.6">
 *          DODAG Configuration
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;                       /**< Option Type: 0x04 */
    uint8_t length;                     /**< length of option, not including first two bytes */
    uint8_t flags_a_pcs;                /**< flags */
    uint8_t dio_int_doubl;              /**< trickle Imax parameter */
    uint8_t dio_int_min;                /**< trickle Imin parameter */
    uint8_t dio_redun;                  /**< trickle k parameter */
    network_uint16_t max_rank_inc;      /**< allowable increase in rank */
    network_uint16_t min_hop_rank_inc;  /**< DAGRank(rank) = floor(rank/MinHopRankIncrease) */
    network_uint16_t ocp;               /**< Objective Code Point */
    uint8_t reserved;                   /**< reserved */
    uint8_t default_lifetime;           /**< lifetime of RPL routes (lifetime * lifetime_unit) */
    network_uint16_t lifetime_unit;     /**< unit in seconds */
} ng_rpl_opt_dodag_conf_t;

/**
 * @brief DODAG Information Solicitation
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.2">
 *          DODAG Information Solicitation
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t flags;      /**< unused */
    uint8_t reserved;   /**< reserved */
} ng_rpl_dis_t;

/**
 * @brief Destination Advertisement Object
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.4">
 *          Destination Advertisement Object
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t instance_id;        /**< id of the instance */
    uint8_t k_d_flags;          /**< K and D flags */
    uint8_t reserved;           /**< reserved */
    uint8_t dao_sequence;       /**< sequence of the DAO, needs to be used for DAO-ACK */
    ipv6_addr_t dodag_id;       /**< id of the DODAG */
} ng_rpl_dao_t;

/**
 * @brief Destination Advertisement Object Acknowledgement
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.5">
 *          Destination Advertisement Object Acknowledgement
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t instance_id;        /**< id of the instance */
    uint8_t d_reserved;         /**< if set, indicates that the DODAG id should be included */
    uint8_t dao_sequence;       /**< sequence must be equal to the sequence from the DAO object */
    uint8_t status;             /**< indicates completion */
    ipv6_addr_t dodag_id;       /**< id of the DODAG */
} ng_rpl_dao_ack_t;

/**
 * @brief Target Option
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.7.7">
 *          RPL Target
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< option type */
    uint8_t length;             /**< option length without the first two bytes */
    uint8_t flags;              /**< unused */
    uint8_t prefix_length;      /**< number of valid leading bits in the IPv6 prefix */
    ipv6_addr_t target;         /**< IPv6 prefix, address or multicast group */
} ng_rpl_opt_target_t;

/**
 * @brief Transit Option
 * @see <a href="https://tools.ietf.org/html/rfc6550#section-6.7.8">
 *          Transit Information
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< option type */
    uint8_t length;             /**< option length without the first two bytes */
    uint8_t e_flags;            /**< external flag indicates external routes */
    uint8_t path_control;       /**< limits the number of DAO parents */
    uint8_t path_sequence;      /**< increased value for route updates */
    uint8_t path_lifetime;      /**< lifetime of routes */
} ng_rpl_opt_transit_t;

typedef struct ng_rpl_dodag ng_rpl_dodag_t;
typedef struct ng_rpl_parent ng_rpl_parent_t;

/**
 * @brief Parent representation
 */
struct ng_rpl_parent {
    ng_rpl_parent_t *next;          /**< pointer to the next parent */
    uint8_t state;                  /**< 0 for unsued, 1 for used */
    ipv6_addr_t addr;               /**< link-local IPv6 address of this parent */
    uint16_t rank;                  /**< rank of the parent */
    uint8_t dtsn;                   /**< last seen dtsn of this parent */
    ng_rpl_dodag_t *dodag;          /**< DODAG the parent belongs to */
    timex_t lifetime;               /**< lifetime of this parent */
    double  link_metric;            /**< metric of the link */
    uint8_t link_metric_type;       /**< type of the metric */
};

/**
 * @brief Objective function representation
 */
typedef struct {
    uint16_t ocp;   /**< objective code point */
    uint16_t (*calc_rank)(ng_rpl_parent_t *parent, uint16_t base_rank); /**< calculate the rank */
    ng_rpl_parent_t *(*which_parent)(ng_rpl_parent_t *, ng_rpl_parent_t *); /**< compare for parents */
    ng_rpl_dodag_t *(*which_dodag)(ng_rpl_dodag_t *, ng_rpl_dodag_t *); /**< compare for dodags */
    void (*reset)(ng_rpl_dodag_t *);    /**< resets the OF */
    void (*parent_state_callback)(ng_rpl_parent_t *, int, int); /**< retrieves the state of a parent*/
    void (*init)(void);  /**< OF specific init function */
    void (*process_dio)(void);  /**< DIO processing callback (acc. to OF0 spec, chpt 5) */
} ng_rpl_of_t;


/**
 * @brief Instance representation
 */
typedef struct {
    uint8_t id;                     /**< id of the instance */
    uint8_t state;                  /**< 0 for unused, 1 for used */
    ng_rpl_dodag_t *dodags;         /**< pointer to the DODAG list of this instance */
    uint8_t mop;                    /**< configured Mode of Operation */
    ng_rpl_of_t *of;                /**< configured Objective Function */
    uint16_t min_hop_rank_inc;      /**< minimum hop rank increase */
    uint16_t max_rank_inc;          /**< max increase in the rank */
} ng_rpl_instance_t;

/**
 * @brief DODAG representation
 */
struct ng_rpl_dodag {
    ng_rpl_instance_t *instance;    /**< id of the instance */
    ng_rpl_dodag_t *next;           /**< pointer to the next dodag */
    ng_rpl_parent_t *parents;       /**< pointer to the parents list of this DODAG */
    ipv6_addr_t dodag_id;           /**< id of the DODAG */
    uint8_t state;                  /**< 0 for unused, 1 for used */
    uint8_t dtsn;                   /**< DAO Trigger Sequence Number */
    uint8_t prf;                    /**< preferred flag */
    uint8_t dio_interval_doubl;     /**< trickle Imax parameter */
    uint8_t dio_min;                /**< trickle Imin parameter */
    uint8_t dio_redun;              /**< trickle k parameter */
    uint8_t default_lifetime;       /**< lifetime of routes (lifetime * unit) */
    uint16_t lifetime_unit;         /**< unit in seconds of the lifetime */
    uint8_t version;                /**< version of this DODAG */
    uint8_t grounded;               /**< grounded flag */
    uint16_t my_rank;               /**< rank/position in the DODAG */
    uint8_t node_status;            /**< leaf, normal, or root node */
    uint8_t dao_seq;                /**< dao sequence number */
    uint8_t dao_counter;            /**< amount of retried DAOs */
    bool dao_ack_received;          /**< flag to check for DAO-ACK */
    uint8_t dodag_conf_counter;     /**< limitation of the sending of DODAG_CONF options */
    timex_t dao_time;               /**< time to schedule the next DAO */
    vtimer_t dao_timer;             /**< timer to schedule the next DAO */
    timex_t cleanup_time;           /**< time to schedula a DODAG cleanup */
    vtimer_t cleanup_timer;         /**< timer to schedula a DODAG cleanup */
    trickle_t trickle;              /**< trickle representation */
};

#ifdef __cplusplus
}
#endif

#endif /* NG_RPL_STRUCTS_H_ */
/**
 * @}
 */