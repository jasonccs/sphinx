/*
Navicat MySQL Data Transfer

Source Server         : 127.0.0.1
Source Server Version : 50726
Source Host           : localhost:3306
Source Database       : test

Target Server Type    : MYSQL
Target Server Version : 50726
File Encoding         : 65001

Date: 2023-04-09 12:23:00
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for documents
-- ----------------------------
DROP TABLE IF EXISTS `documents`;
CREATE TABLE `documents` (
                             `id` int(11) NOT NULL AUTO_INCREMENT,
                             `group_id` int(11) NOT NULL,
                             `group_id2` int(11) NOT NULL,
                             `date_added` datetime NOT NULL,
                             `title` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
                             `content` text COLLATE utf8mb4_unicode_ci NOT NULL,
                             PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- Records of documents
-- ----------------------------
INSERT INTO `documents` VALUES ('1', '1', '5', '2023-04-08 21:18:40', 'test one', 'this is my test document number one. also checking search within phrases.');
INSERT INTO `documents` VALUES ('2', '1', '6', '2023-04-08 21:18:40', 'test two 中国', 'this is my test document number doc 湖南');
INSERT INTO `documents` VALUES ('3', '2', '7', '2023-04-08 21:18:40', 'another doc 湖南株洲', 'this is another 中国 你好，湖北武汉');
INSERT INTO `documents` VALUES ('4', '2', '8', '2023-04-08 21:18:40', 'doc number four 湖北武汉', 'this is to test groups 湖北武汉');
INSERT INTO `documents` VALUES ('5', '2', '8', '2023-04-08 21:18:40', 'doc number four 湖北武汉', 'this is to test groups 湖北武汉');

-- ----------------------------
-- Table structure for tags
-- ----------------------------
DROP TABLE IF EXISTS `tags`;
CREATE TABLE `tags` (
                        `docid` int(11) NOT NULL,
                        `tagid` int(11) NOT NULL,
                        `content` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
                        UNIQUE KEY `docid` (`docid`,`tagid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- Records of tags
-- ----------------------------
INSERT INTO `tags` VALUES ('1', '1', '湖北武汉');
INSERT INTO `tags` VALUES ('1', '3', 'doc 武汉');
INSERT INTO `tags` VALUES ('1', '5', 'doc 武汉');
INSERT INTO `tags` VALUES ('1', '7', '湖南株洲');
INSERT INTO `tags` VALUES ('2', '6', '上海');
INSERT INTO `tags` VALUES ('2', '4', '武汉 上海');
INSERT INTO `tags` VALUES ('2', '2', '上海武汉');
INSERT INTO `tags` VALUES ('3', '15', 'number ');
INSERT INTO `tags` VALUES ('4', '7', 'number ');
INSERT INTO `tags` VALUES ('4', '40', 'number ');
