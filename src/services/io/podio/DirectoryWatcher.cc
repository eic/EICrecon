
#include "DirectoryWatcher.h"

#include <filesystem>
#include <iostream>

void DirectoryWatcher::Update() {

    std::map<std::filesystem::file_time_type, std::filesystem::path> new_files;

    // Retrieve directory listing
    for (const auto& entry : std::filesystem::directory_iterator(m_directory_to_watch)) {

        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().filename().string().starts_with('.')) {
            // Skip dotfiles. These are probably files still being copied
            continue;
        }

        auto existing = m_directory_contents.find(entry.path().string());
        if (existing != m_directory_contents.end()) {
            continue;
        }

        new_files[entry.last_write_time()] = entry.path().string();
    }

    // Add to the back of the 'ready' queue in sorted order
    for (auto& [time, path] : new_files) {
        m_ready_filenames.push(path);
        m_directory_contents[path.string()] = Status::Ready;
        std::cout << "Watcher found file " << path << std::endl;
    }
}


std::optional<std::string> DirectoryWatcher::GetNextFilename() {
    Update();
    if (m_ready_filenames.empty()) {
        return std::nullopt;
    }
    auto filename = m_ready_filenames.front();
    m_ready_filenames.pop();
    m_directory_contents[filename] = Status::Processing;
    std::cout << "Watcher is emitting file " << filename << std::endl;
    return filename;
}

void DirectoryWatcher::FinishSuccess(const std::string& filename) {

    std::error_code ec;
    bool success = std::filesystem::remove(filename, ec);

    if (!success) {
        // If delete fails, we need to remember this file so we don't re-process it
        m_directory_contents.at(filename) = Status::FinishedSuccess;
        std::cout << "Failed to delete " << filename << ": " << ec.message() << std::endl;
    }
    else {
        m_directory_contents.erase(filename);
        std::cout << "Deleted finished file " << filename << std::endl;
    }

}

void DirectoryWatcher::FinishFailure(const std::string& filename) {

    // We don't delete the file if it fails because we may want to inspect it
    m_directory_contents.at(filename) = Status::FinishedFailure;
    std::cout << "Ignoring failing file " << filename << std::endl;
}
