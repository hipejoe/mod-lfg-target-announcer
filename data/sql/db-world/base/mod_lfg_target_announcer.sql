DROP TABLE IF EXISTS `mod_lfg_target_announcer`;

CREATE TABLE `mod_lfg_target_announcer`
(
    `map_id` INT UNSIGNED NOT NULL,
    `message` VARCHAR(1000) NOT NULL,
    `comment` VARCHAR(255) NULL,
    PRIMARY KEY (`map_id`)
)
ENGINE = InnoDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_unicode_ci;

INSERT INTO `mod_lfg_target_announcer`
(
    `map_id`,
    `message`,
    `comment`
)
VALUES
(
    601,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Mal''Ganis. The Infinite Corruptor is optional.',
    'The Culling of Stratholme'
),
(
    649,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Argent Confessor Paletress or Eadric the Pure, followed by the Black Knight.',
    'Trial of the Champion'
),
(
    658,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must complete the Lich King escape encounter.',
    'Halls of Reflection'
),
(
    608,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Cyanigosa after completing wave 18.',
    'The Violet Hold'
),
(
    600,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat The Prophet Tharon''ja. King Dred is optional.',
    'Drak''Tharon Keep'
),
(
    604,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Gal''darah. Eck the Ferocious is optional.',
    'Gundrak'
),
(
    599,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Sjonnir the Ironshaper.',
    'Halls of Stone'
),
(
    602,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Loken.',
    'Halls of Lightning'
),
(
    576,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Keristrasza.',
    'The Nexus'
),
(
    578,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Ley-Guardian Eregos.',
    'The Oculus'
),
(
    574,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat Ingvar the Plunderer.',
    'Utgarde Keep'
),
(
    575,
    '|cff00ff00[LFG System]:|r To complete this dungeon and claim your daily rewards, you must defeat King Ymiron.',
    'Utgarde Pinnacle'
);