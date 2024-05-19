# Game Design Document

## Section 0: Summary
This section should tell the reader what to expect in the future sections.  In particular, it should contain at least:
- a working title for your game
- a list of your team members (and their roles if decided)
- a "concept statement" of your game ("your game in a tweet")

## Section 1: Gameplay
This section should address simple questions about how your game works:
- How does your game progress?
- What are the win and loss conditions?
- Are there levels?
- Are there points?

This section should also address:
- **controls** (keyboard? mouse? tongue?)
- **physics** (how does your game incorporate the physics engine?)
- **game flow** (what does the game look like from start to end for the player?)
- **graphics** (will you draw polygons? use sprites? make your own vector graphics?)

## Section 2: Feature Set
This section should reduce your game to a set of individual features (remember iterative development?).  Your team should
assign these features to individual group members and assign each one a priority from one to four (1 = game cannot work without this, 4 = cool but not necessary).

We have gathered together the following list of some example features you might choose to implement:
- make your own graphics or sprites
- add sound effects
- implement a scrolling environment
- implement a networked/multiplayer game
- implement 2D parallax (https://en.wikipedia.org/wiki/Parallax)
- implement rendering of text
- implement a mouse handler
- implement an AI for an enemy
- implement speed-independent friction
- implement more accurate integration (current implementation uses a trapezoid sum)
- implement music

## Section 3: Timeline
This section should assign every feature in the previous section to a particular group member and a particular week they will implement it.

## Section 4: Disaster Recovery
Firstly, we will each develop a personalized plan to outline specific steps to take should we fall behind. This could include dedicating extra time to the project and adjusting priorities, but most importantly seeking help from teammates and TAs. By proactively seeking assistance from teammates if we encounter challenges, we can leverage the collective knowledge and expertise of the team to hasten problem solving.

Secondly, we will regularly meet up so that we can check in with each other. During these meetings, we will evaluate and track progress on the current tasks. If our expectations turn out to be unrealistic, we have sufficient flexibility so that we can discuss redistributing tasks or adjusting the timeline. Furthermore, in these meetings, we can obtain feedback on our current work and identify areas for improvement, if necessary.

Lastly, we will encourage frequent testing and commits to identify and fix errors earlier, improve code quality, and prevent catastrophic code failures caused by large, untested changes. By saving and committing to Git on a semi-frequent basis, we can ensure that we can collaborate effectively by facilitating the integration of new features and fixes into the main code as well as secure a functional version of the game that we can revert to in case a new feature causes previous implementations to fail.
