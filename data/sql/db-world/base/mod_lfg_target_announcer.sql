DROP TABLE IF EXISTS `mod_lfg_target_announcer`;

CREATE TABLE `mod_lfg_target_announcer`
(
    `lfg_dungeon_id` INT UNSIGNED NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `message` VARCHAR(1000) NOT NULL,
    `comment` VARCHAR(255) NULL,
    PRIMARY KEY (`lfg_dungeon_id`),
    INDEX `IX_mod_lfg_target_announcer_map_id` (`map_id`)
)
ENGINE = InnoDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `mod_lfg_target_announcer_target`;

CREATE TABLE `mod_lfg_target_announcer_target`
(
    `lfg_dungeon_id` INT UNSIGNED NOT NULL,
    `target_order` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `creature_entry` INT UNSIGNED NULL,
    `target_name` VARCHAR(100) NOT NULL,
    `required` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `comment` VARCHAR(255) NULL,

    PRIMARY KEY
    (
        `lfg_dungeon_id`,
        `target_order`
    ),

    INDEX `IX_mod_lfg_target_creature_entry`
        (`creature_entry`),

    CONSTRAINT `FK_mod_lfg_target_announcer_target_dungeon`
        FOREIGN KEY (`lfg_dungeon_id`)
        REFERENCES `mod_lfg_target_announcer` (`lfg_dungeon_id`)
        ON DELETE CASCADE
)
ENGINE = InnoDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_unicode_ci;

DROP PROCEDURE IF EXISTS `AddLfgCreatureTarget`;

DELIMITER //

CREATE PROCEDURE `AddLfgCreatureTarget`
(
    IN `p_lfg_dungeon_id` INT UNSIGNED,
    IN `p_target_order` TINYINT UNSIGNED,
    IN `p_creature_entry` INT UNSIGNED,
    IN `p_required` TINYINT UNSIGNED,
    IN `p_comment` VARCHAR(255)
)
BEGIN
    INSERT INTO `mod_lfg_target_announcer_target`
    (
        `lfg_dungeon_id`,
        `target_order`,
        `creature_entry`,
        `target_name`,
        `required`,
        `comment`
    )
    SELECT
        `p_lfg_dungeon_id`,
        `p_target_order`,
        `ct`.`entry`,
        `ct`.`name`,
        `p_required`,
        `p_comment`
    FROM `creature_template` AS `ct`
    WHERE `ct`.`entry` = `p_creature_entry`;
END//

DELIMITER ;