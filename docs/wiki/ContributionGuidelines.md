# Contribution Guidelines

We welcome and appreciate contributions to the R-Type project! By contributing, you help make this project better for everyone. Please take a moment to review this document to understand how to contribute effectively.

## How to Contribute

1.  **Fork the Repository**: Start by forking the main R-Type repository to your GitHub account.
2.  **Clone Your Fork**: Clone your forked repository to your local machine.
    ```bash
    git clone https://github.com/YourUsername/CPP-Advanced_C---R-Type.git
    cd CPP-Advanced_C---R-Type
    ```
3.  **Create a New Branch**: Create a new branch for your feature or bug fix. Use a descriptive name (e.g., `feature/add-new-enemy-type`, `bugfix/fix-player-collision`).
    ```bash
    git checkout -b your-branch-name
    ```
4.  **Make Your Changes**: Implement your feature or fix the bug. Ensure your code adheres to the project's coding style and conventions.
5.  **Test Your Changes**: If applicable, add or update unit tests to cover your changes. Ensure all existing tests pass.
6.  **Commit Your Changes**: Write clear, concise commit messages. A good commit message explains *what* was changed and *why*.
    ```bash
    git commit -m "feat: Add new enemy type with unique attack pattern"
    ```
7.  **Push to Your Fork**: Push your local branch to your forked repository on GitHub.
    ```bash
    git push origin your-branch-name
    ```
8.  **Create a Pull Request (PR)**: Go to the original R-Type repository on GitHub and create a new Pull Request from your forked branch. Provide a detailed description of your changes, including any relevant issue numbers.

## Coding Style and Conventions

*   **Follow Existing Style**: Please adhere to the existing coding style and conventions found in the codebase. This includes naming conventions, indentation, brace style, and comment formatting.
*   **C++17 Standard**: All new code should be compatible with the C++17 standard.
*   **Clear and Readable Code**: Write code that is easy to understand and maintain.
*   **Comments**: Add comments where necessary to explain complex logic or non-obvious decisions. Avoid commenting on obvious code.

## Testing

*   **Unit Tests**: For new features or bug fixes, please include relevant unit tests. This helps ensure the stability and correctness of the codebase.
*   **Build System**: Ensure your changes build successfully using CMake and Conan.

## Pull Request Guidelines

*   **Descriptive Title**: Your PR title should briefly summarize the changes.
*   **Detailed Description**: Provide a clear and concise description of your changes. Explain the problem your PR solves, how it solves it, and any potential impacts.
*   **Refer to Issues**: If your PR addresses an existing issue, please link to it (e.g., `Fixes #123`, `Closes #456`).
*   **Code Review**: Be prepared for constructive feedback during the code review process. Address comments and make necessary adjustments.

## Reporting Bugs

If you find a bug, please open an issue on the GitHub issue tracker. Provide as much detail as possible, including:

*   A clear and concise description of the bug.
*   Steps to reproduce the behavior.
*   Expected behavior.
*   Screenshots or error messages, if applicable.
*   Your operating system and compiler version.

Thank you for your contributions!