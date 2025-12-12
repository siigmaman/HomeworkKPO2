CREATE TABLE IF NOT EXISTS works (
    id SERIAL PRIMARY KEY,
    student_id VARCHAR(100) NOT NULL,
    student_name VARCHAR(255) NOT NULL,
    assignment_id VARCHAR(100) NOT NULL,
    assignment_name VARCHAR(255),
    file_path VARCHAR(500) NOT NULL,
    original_filename VARCHAR(255) NOT NULL,
    file_hash VARCHAR(64) UNIQUE NOT NULL,
    upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(50) DEFAULT 'uploaded'
);

CREATE TABLE IF NOT EXISTS reports (
    id SERIAL PRIMARY KEY,
    work_id INTEGER NOT NULL REFERENCES works(id) ON DELETE CASCADE,
    analysis_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    plagiarism_found BOOLEAN DEFAULT FALSE,
    similarity_percentage DECIMAL(5,2) DEFAULT 0.00,
    matched_work_id INTEGER REFERENCES works(id),
    matched_student_name VARCHAR(255),
    report_data JSONB,
    status VARCHAR(50) DEFAULT 'completed'
);

CREATE INDEX IF NOT EXISTS idx_works_file_hash ON works(file_hash);
CREATE INDEX IF NOT EXISTS idx_works_student_assignment ON works(student_id, assignment_id);
CREATE INDEX IF NOT EXISTS idx_reports_work_id ON reports(work_id);
CREATE INDEX IF NOT EXISTS idx_works_status ON works(status);