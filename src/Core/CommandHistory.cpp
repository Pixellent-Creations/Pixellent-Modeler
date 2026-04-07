#include "PixellentModeler/Core/CommandHistory.hpp"
#include "PixellentModeler/Core/Log.hpp"

namespace PixellentModeler {

void CommandHistory::execute(std::unique_ptr<Command> cmd) {
    cmd->execute();
    Log::info("Executed: " + cmd->description());
    m_undoStack.push_back(std::move(cmd));

    // Clear the redo stack whenever a new command is executed
    m_redoStack.clear();

    // Enforce maximum history size
    if (m_undoStack.size() > MAX_HISTORY) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

void CommandHistory::undo() {
    if (!canUndo()) {
        Log::warn("Nothing to undo");
        return;
    }

    auto& cmd = m_undoStack.back();
    cmd->undo();
    Log::info("Undone: " + cmd->description());
    m_redoStack.push_back(std::move(cmd));
    m_undoStack.pop_back();
}

void CommandHistory::redo() {
    if (!canRedo()) {
        Log::warn("Nothing to redo");
        return;
    }

    auto& cmd = m_redoStack.back();
    cmd->execute();
    Log::info("Redone: " + cmd->description());
    m_undoStack.push_back(std::move(cmd));
    m_redoStack.pop_back();
}

bool CommandHistory::canUndo() const {
    return !m_undoStack.empty();
}

bool CommandHistory::canRedo() const {
    return !m_redoStack.empty();
}

std::string CommandHistory::undoDescription() const {
    if (!canUndo()) return "";
    return m_undoStack.back()->description();
}

std::string CommandHistory::redoDescription() const {
    if (!canRedo()) return "";
    return m_redoStack.back()->description();
}

void CommandHistory::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
    Log::info("Command history cleared");
}

} // namespace PixellentModeler
