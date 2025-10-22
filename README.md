# defend-the-dojo
* This is a program made in Raylib C++. It is a small, fun game made in C++.
* The Player is a green ninja defending the dojo with all their power.
* The Player weilds a spear that they use to defend the dojo against the attackers.
* The attackers are red ninjas that spawn in packs.
* The attackers throw ninja stars against the Player in a cooldown of 1-2 seconds and try to defeat the Player.
* The attackers first move for 150 pixels and then stop, and move horizontally in their place, throwing ninja stars accurately at the Player.
* The attackers each have a maximum of 20 HP, and spawn randomly from 1 to 5 in each round.
* Each ninja star of the attackers' can do randomly from 3-5 HP of damage to the player.
* The Player can choose their desired game mode in the title screen of the game.
* There are three game modes in this game:
  - Peaceful mode: In this game mode, the player's health does not decrease with ninja star hits. It is meant for users who want to practice playing the game.
  - Easy mode: In this game mode, the Player has a maximum HP of 200 HP. It is good for casual Players.
  - Medium mode: In this game mode, the Player has a maximum HP of 150 HP. It is good for intermediate Players.
  - Hard mode: In this game mode, the Player has a maximum HP of 100 HP. It is good for skilled Players.
* The Player can move up, down, left, and right.
* The Player has an attack ability, which is called Spear Dash.
* The Player can aim their spear at an attacker, and dash at them.
* When the Player dashes, they accelerate quickly towards the direction of their spear.
* During the Player dash, the Dash Time counter decreases, and the Dash Power counter increases.
* When the Player hits the attacker when the Dash Power is high, the Player does more damage to the attacker. Therefore, it is better to time the attacks, and move a distance away from the attacker and then dash.
* The Player can also hit multiple attackers at once in one dash.
* When the Player reduces an attacker's HP to 0, the attacker dies, and the score increments.
* When the Dash Time counter is 0, the Player will stop dashing.
* When the Player stops dashing, the Dash Time counter slowly increases, and the Dash Power counter is automatically set to 0.
* The Player can also unlock multiple spears:
  - Iron Spear: Default.
  - Golden Spear: Score is greater than 10.
  - Diamond Spear: Score is greater than 20.
* The Spear does damage according to the Dash Power and the maximum attack damage of the Spear type:
  - Iron Spear: Maximum – 10 HP.
  - Golden Spear: Maximum – 13 HP.
  - Diamond Spear: Maximum – 16 HP.
  - Ruby Spear: Maximum – 20 HP.
* The Spears can be unlocked from the score counter increasing:
  - Default: Iron Spear (score >= 0).
  - Score > 10: Golden Spear.
  - Score > 20: Diamond Spear.
  - Score > 50: Ruby Spear.
* The Spears can be interchanged using the Left Alt key.
* The Player also has an ability called Lure.
* The Lure ability has a 10 second cooldown.
* The Lure ability forces the attackers to take 50 small steps towards the player.
* This ability makes it easier for the Player to kill the attackers, as the attackers group up when the Lure ability is used.
* The Player also has an option to purchase Mystery Boxes.
* The Purchase button is unlocked when the Player's score is more than 25.
* When the Purchase button is pressed, a Mystery Box spawns in the dojo, for the cost of 5 points.
* The Mystery Boxes can randomly have these effects when the Player collects them:
  - Immunity: The Player does not take any damage from ninja stars.
  - Extra Speed: The Player's moving speed, dashing speed, and regaining of dash time is increased, along with the dash power counter increasing faster.
  - Extra Damage: The Player's dash ability does more damage to attackers.
* Each of these effects last for 10 seconds.
* The User can also pause and play the game using the Pause/Play Button in the bottom-left corner.
* If the player's health goes to 0, then the player dies, and the user returns to the title screen, and in the next game, all stats are reset.
* The controls of the game are:
  - Move Up: Key W / Key Up.
  - Move Down: Key S / Key Down.
  - Move Left: Key A / Key Left.
  - Move Right: Key D / Key Right.
  - Spear Dash: Key Space.
  - Lure: Key E / Key Right Shift.
  - End Game: Key Period.
* The Player's goal is to dodge the ninja stars, use dash ability effectively to kill attackers, use Lure ability with proper judgement, unlock all types of spears, purchase Mystery Boxes whenever necessary, and enjoy the game!
* The repository also has a builtin version of the game, where all the media such as fonts, audio, and images are built into the executable file.
