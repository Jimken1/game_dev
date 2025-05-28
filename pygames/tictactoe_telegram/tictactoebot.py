import logging
import random
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext import Application, CommandHandler, CallbackQueryHandler, ContextTypes

# Enable logging
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)

# Game state storage - using a dictionary to store active games
# Key: chat_id, Value: game state
games = {}

class TicTacToeGame:
    def __init__(self, user_id):
        self.board = [' ' for _ in range(9)]
        self.current_player = 'X'  # User always plays as X
        self.user_id = user_id
        self.game_over = False
        self.winner = None
    
    def make_move(self, position):
        """Make a move on the board."""
        if self.board[position] == ' ' and not self.game_over:
            self.board[position] = self.current_player
            
            # Check if the game is over
            if self.check_winner():
                self.game_over = True
                self.winner = self.current_player
                return True
            
            # Check for draw
            if ' ' not in self.board:
                self.game_over = True
                self.winner = 'Draw'
                return True
            
            # Switch player
            self.current_player = 'O' if self.current_player == 'X' else 'X'
            return True
        return False
    
    def check_winner(self):
        """Check if there's a winner."""
        # Define winning combinations
        win_combinations = [
            [0, 1, 2], [3, 4, 5], [6, 7, 8],  # Rows
            [0, 3, 6], [1, 4, 7], [2, 5, 8],  # Columns
            [0, 4, 8], [2, 4, 6]              # Diagonals
        ]
        
        for combo in win_combinations:
            if self.board[combo[0]] != ' ' and self.board[combo[0]] == self.board[combo[1]] == self.board[combo[2]]:
                return True
        return False
    
    def ai_move(self):
        """Simple AI to make a move."""
        if self.game_over or self.current_player != 'O':
            return False
        
        # Try to win
        position = self.find_winning_move('O')
        if position is not None:
            return self.make_move(position)
        
        # Block user from winning
        position = self.find_winning_move('X')
        if position is not None:
            return self.make_move(position)
        
        # Take center if available
        if self.board[4] == ' ':
            return self.make_move(4)
        
        # Take a corner if available
        corners = [0, 2, 6, 8]
        random.shuffle(corners)
        for corner in corners:
            if self.board[corner] == ' ':
                return self.make_move(corner)
        
        # Take any available edge
        edges = [1, 3, 5, 7]
        random.shuffle(edges)
        for edge in edges:
            if self.board[edge] == ' ':
                return self.make_move(edge)
        
        return False
    
    def find_winning_move(self, player):
        """Find a winning move for the given player."""
        # Define winning combinations
        win_combinations = [
            [0, 1, 2], [3, 4, 5], [6, 7, 8],  # Rows
            [0, 3, 6], [1, 4, 7], [2, 5, 8],  # Columns
            [0, 4, 8], [2, 4, 6]              # Diagonals
        ]
        
        for combo in win_combinations:
            # Count pieces in the combo
            count = 0
            empty_pos = -1
            
            for pos in combo:
                if self.board[pos] == player:
                    count += 1
                elif self.board[pos] == ' ':
                    empty_pos = pos
            
            # If there are two pieces and one empty spot, we can win/block
            if count == 2 and empty_pos != -1:
                return empty_pos
        
        return None

def get_board_markup(game):
    """Create an inline keyboard representing the board."""
    keyboard = []
    for i in range(0, 9, 3):
        row = []
        for j in range(3):
            pos = i + j
            text = game.board[pos] if game.board[pos] != ' ' else ' '
            row.append(InlineKeyboardButton(text, callback_data=f'move_{pos}'))
        keyboard.append(row)
    
    # Add a restart button at the bottom
    keyboard.append([InlineKeyboardButton("Restart Game", callback_data='restart')])
    return InlineKeyboardMarkup(keyboard)

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /start is issued."""
    user = update.effective_user
    await update.message.reply_text(
        f"Hi {user.first_name}! I'm a Tic-Tac-Toe bot. Use /play to start a game!"
    )

async def play(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Start a new Tic-Tac-Toe game."""
    user_id = update.effective_user.id
    chat_id = update.effective_chat.id
    
    # Create a new game
    games[chat_id] = TicTacToeGame(user_id)
    
    await update.message.reply_text(
        "Let's play Tic-Tac-Toe! You are X, I am O. Click to make your move:",
        reply_markup=get_board_markup(games[chat_id])
    )

async def button_handler(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Handle button presses."""
    query = update.callback_query
    await query.answer()
    
    chat_id = update.effective_chat.id
    user_id = update.effective_user.id
    
    # Check if there's an active game
    if chat_id not in games:
        await query.edit_message_text(
            "No active game. Use /play to start a new one!"
        )
        return
    
    game = games[chat_id]
    
    # Handle restart
    if query.data == 'restart':
        games[chat_id] = TicTacToeGame(user_id)
        await query.edit_message_text(
            "Game restarted! You are X, I am O. Click to make your move:",
            reply_markup=get_board_markup(games[chat_id])
        )
        return
    
    # Handle move
    if query.data.startswith('move_'):
        position = int(query.data.split('_')[1])
        
        # Ignore if it's not the user's turn or the game is over
        if game.current_player != 'X' or game.game_over:
            await query.edit_message_text(
                "It's not your turn or the game is over!",
                reply_markup=get_board_markup(game)
            )
            return
        
        # Make the user's move
        if game.make_move(position):
            status_text = "Tic-Tac-Toe"
            
            # Check if the game is over after user's move
            if game.game_over:
                if game.winner == 'X':
                    status_text = "You won! 🎉"
                elif game.winner == 'Draw':
                    status_text = "It's a draw! 🤝"
            else:
                # AI makes its move
                game.ai_move()
                
                # Check if the game is over after AI's move
                if game.game_over:
                    if game.winner == 'O':
                        status_text = "I won! Better luck next time. 🤖"
                    elif game.winner == 'Draw':
                        status_text = "It's a draw! 🤝"
            
            await query.edit_message_text(
                f"{status_text}\nYou: X | Bot: O",
                reply_markup=get_board_markup(game)
            )
        else:
            await query.answer("Invalid move!")

def main() -> None:
    """Start the bot."""
    # Replace 'YOUR_BOT_TOKEN' with your actual token from BotFather
    token = "YOUR_BOT_TOKEN"
    
    # Create the Application
    application = Application.builder().token(token).build()
    
    # Add handlers
    application.add_handler(CommandHandler("start", start))
    application.add_handler(CommandHandler("play", play))
    application.add_handler(CallbackQueryHandler(button_handler))
    
    # Run the bot until the user presses Ctrl-C
    application.run_polling()

if __name__ == '__main__':
    main()