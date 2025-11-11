-- Создание тестовой базы данных
CREATE DATABASE IF NOT EXISTS test_mysql_stability;
USE test_mysql_stability;

-- Таблица с основными типами данных
CREATE TABLE test_data_types (
    id INT PRIMARY KEY AUTO_INCREMENT,
    tiny_int_col TINYINT,
    small_int_col SMALLINT,
    medium_int_col MEDIUMINT,
    int_col INT,
    big_int_col BIGINT,
    decimal_col DECIMAL(10,2),
    float_col FLOAT,
    double_col DOUBLE,
    char_col CHAR(10),
    varchar_col VARCHAR(100),
    text_col TEXT,
    blob_col BLOB,
    date_col DATE,
    time_col TIME,
    datetime_col DATETIME,
    timestamp_col TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    year_col YEAR,
    json_col JSON,
    bool_col BOOLEAN,
    enum_col ENUM('active','inactive','pending')
);

-- Таблица для тестирования внешних ключей
CREATE TABLE test_related (
    rel_id INT PRIMARY KEY AUTO_INCREMENT,
    main_id INT,
    description VARCHAR(50),
    FOREIGN KEY (main_id) REFERENCES test_data_types(id) ON DELETE CASCADE
);

-- Вставка данных
INSERT INTO test_data_types VALUES (
    NULL, 127, 32000, 8388607, 2147483647, 9223372036854775807,
    1234567.89, 3.14, 2.71828, 'fixed', 'variable text',
    'Long text content', 'binary data', '2024-01-15', '14:30:00',
    '2024-01-15 14:30:00', NULL, 2024,
    '{"key": "value", "array": [1,2,3]}', TRUE, 'active'
);

INSERT INTO test_related VALUES (NULL, 1, 'Related record 1');

-- SELECT операции
SELECT * FROM test_data_types;
SELECT id, varchar_col, json_col FROM test_data_types WHERE bool_col = TRUE;
SELECT t.*, r.description FROM test_data_types t 
JOIN test_related r ON t.id = r.main_id;

-- UPDATE операции
UPDATE test_data_types SET varchar_col = 'updated text' WHERE id = 1;
UPDATE test_related SET description = CONCAT(description, ' - modified');

-- DELETE операция
INSERT INTO test_related VALUES (NULL, 1, 'To be deleted');
DELETE FROM test_related WHERE description LIKE '%deleted%';

-- ALTER таблицы
ALTER TABLE test_data_types ADD COLUMN new_column VARCHAR(50) DEFAULT 'new default';
ALTER TABLE test_data_types MODIFY COLUMN varchar_col VARCHAR(200);
ALTER TABLE test_data_types ADD INDEX idx_varchar (varchar_col);

-- Дополнительные INSERT после ALTER
INSERT INTO test_data_types VALUES (
    NULL, -128, -32768, -8388608, -2147483648, -9223372036854775808,
    -987654.32, -1.5, -2.5, 'negative', 'negative values test',
    'Negative content', NULL, '2023-12-31', '23:59:59',
    '2023-12-31 23:59:59', NULL, 2023,
    '{"negative": true, "values": [-1,-2,-3]}', FALSE, 'inactive'
);

-- Проверка JSON операций
SELECT json_col->>'$.key' AS json_value FROM test_data_types;
UPDATE test_data_types SET json_col = JSON_SET(json_col, '$.new_key', 'new_value');

-- Очистка (раскомментировать для финальной проверки)
-- DROP DATABASE test_mysql_stability;
