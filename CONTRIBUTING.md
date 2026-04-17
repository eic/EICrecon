# Contribution Guidelines

EICrecon is a collaborative software project, and contributions from anyone are welcome.
This document describes the recommended workflow for developing and reviewing contributions to EICrecon. Questions about any step of the process can be directed to the review team, which can be contacted via email:

Email: epic-software-review@lists.bnl.gov

## How to contribute

The following steps describe the recommended workflow for contributing to EICrecon.


```mermaid
flowchart LR
    A(["Plan/Communicate"])
    A --> B(["Create draft PR"])
      --> C(["Develop self-contained commits"])
      --> D(["Ready for review"])
      --> E(["Timescale for review"])
```

1. Communicate your development plans with the Reconstruction Framework and Algorithms or Physics and Detector Simulations Working Groups before beginning development. This helps clarify the scope and expected timeline.

2. Create a draft PR as soon as work begins. You may request a reviewer directly, or ask the review team to assign one. If the assigned reviewer cannot complete the review, they should request a new reviewer from the review team.

3. Develop your code using concise commits with descriptive commit messages.

    #### Development workflow

    The following recommendations describe how to develop a contribution within EICrecon

    * Start each development in a new branch.
    * Use atomic commits, and commit frequently.

        - Each commit should reflect a single self-contained change. Try to avoid overly large commits (bad examples are for instance mixing logical change with code cleanup and typo fixes).

    * Write descriptive commit messages to preserve development history. See e.g. [here](https://wiki.openstack.org/wiki/GitCommitMessages#Information_in_commit_messages) for more information.
    * Try to keep PRs as small as possible, so that each PR belongs to one logically/conceptually different development. This simplifies the code review process.

4. When the code is ready for review, mark the pull request as **Ready for Review** to notify the reviewer.

5. The reviewer should provide a timescale for their review without delay during working hours (depending on the scope and size of the PR and on collaboration schedule).

### PR checklist

A PR is ready to be reviewed when:

- [x] A detailed description of the PR is provided
- [x] All CI jobs pass
- [x] clang-tidy and clang-format have been run locally (see [link](some link) for instructions)
- [x] There is a legible commit history
- [x] Code is appropriately documented

## Reviewing other contributions

Upon the code being marked ready for review, the review period for the PR begins. ePIC requires every PR of EICrecon receive at least one review prior to being marked ready to be merged. Reviewers and all collaborators of ePIC are expected to abide by the ePIC [code of conduct](https://www.epic-eic.org/collaboration/professional.html). The reviewer should evaluate the following criteria

#### Approving a pull request

- Does the PR have a description that reflects the content of the code?
- Does the CI pass?
- Is the contributed code:
    - maintainable and sustainable?
    - (if applicable) correct in satisfying physics requirements and maintaining downstream assumptions?
    - scalable (i.e. it can run in production)?
- Are relevant performance benchmarks considered? And if changes in computing and/or physics performance are observed, are they discussed and understood?
- Have all comments raised by collaborators/reviewers been addressed?

## Code style and standards

EICrecon uses clang-format for formatting the source code. Your code can be formatted automatically using the .clang-format file
- class names: PascalCase
- Base namespace: eicrecon
- Indent: 2 spaces
- File naming:

    - File extensions: .cc, .h
    - Algorithm config name: <AlgorithmName>Config.h
    - Factory name: <AlgorithmName>_factory.h
    - Templated classes end with "T" (classes inheriting from them should not use the T suffix unless they are also templates)
- For more details see [.clang-format](https://github.com/eic/EICrecon/blob/main/.clang-format)
