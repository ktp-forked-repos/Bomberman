#include "Game.h"


Game::Game(size_t width, size_t height)
{
	m_windowWidth = width;
	m_windowHeight = height;
	m_window.create(sf::VideoMode(static_cast<int>(m_windowWidth), static_cast<int>(m_windowHeight)), "Bomberman | Created by PiGames", sf::Style::Close);
	m_window.setFramerateLimit(60);

	//hmmm /Conrad
	Player p;
	m_players.push_back(p);
	m_players.push_back(p);
}


Game::~Game()
{
}


void Game::Run()
{
	/* TMP INIT BEGIN*/
	const int RESOURCE_COUNT = 6;
	std::string resourcePaths[RESOURCE_COUNT] =
	{
		"data/sample_level.txt",
		"data/sample_terraintextures.png",
		"data/sample_playertextures.png",
		"data/sample_bombtextures.png",
		"data/sample_raytextures.png",
		"data/sample_playertextures2.png"
	};
	if (!m_level.LoadFromFile(resourcePaths[0]))
	{
		std::cerr << "[!] Cannot load file: \"" << resourcePaths[0] << "\". Exiting...\n";
		std::exit(1);
	}

	// loading resources
	if (!m_atlasTerrain.LoadFromFile(resourcePaths[1]))
	{
		std::cerr << "[!] Cannot load resource: '" << resourcePaths[1] << std::endl;
		std::exit(1);
	}

	if (!m_atlasPlayer.LoadFromFile(resourcePaths[2]))
	{
		std::cerr << "[!] Cannot load resource: '" << resourcePaths[2] << std::endl;
		std::exit(1);
	}

	if (!m_atlasBomb.LoadFromFile(resourcePaths[3]))
	{
		std::cerr << "[!] Cannot load resource: '" << resourcePaths[3] << std::endl;
		std::exit(1);
	}
	if (!m_atlasBombRay.LoadFromFile(resourcePaths[4]))
	{
		std::cerr << "[!] Cannot load resource: '" << resourcePaths[4] << std::endl;
		std::exit(1);
	}
	if (!m_atlasPlayer2.LoadFromFile(resourcePaths[5]))
	{
		std::cerr << "[!] Cannot load resource: '" << resourcePaths[5] << std::endl;
		std::exit(1);
	}

	// setting up resources
	m_atlasTerrain.TrimByGrid(64, 64);
	m_atlasPlayer.TrimByGrid(32, 32);
	m_atlasPlayer2.TrimByGrid(32, 32);
	m_atlasBomb.TrimByGrid(64, 64);
	m_atlasBombRay.TrimByGrid(64, 64);

	m_levelView.SetLevel(&m_level, &m_atlasTerrain);
	m_level.SetLevelView(&m_levelView);


	// setting up player
	Animator playerAnimator;
	playerAnimator.AddAnimationState("default", m_atlasPlayer, 0, m_atlasPlayer.GetCount() - 1);
	playerAnimator.SetLoop(true);

	m_players[0].SetAnimator(playerAnimator, m_atlasPlayer.GetCellSizeX(), m_atlasPlayer.GetCellSizeY());
	playerAnimator.ChangeActiveState("default");

	// setting up player2
	Animator playerAnimator2;
	playerAnimator2.AddAnimationState("default", m_atlasPlayer2, 0, m_atlasPlayer2.GetCount() - 1);
	playerAnimator2.SetLoop(true);

	m_players[1].SetAnimator(playerAnimator2, m_atlasPlayer2.GetCellSizeX(), m_atlasPlayer2.GetCellSizeY());
	playerAnimator2.ChangeActiveState("default");

	
	// setting up bomb (and something for physic engine) 
	std::map<int, PhysicalBody*> players;
	for (unsigned i = 0; i < m_players.size(); ++i)
	{
		m_players[i].SetUpBomb(&m_atlasBomb, &m_atlasBombRay);
		m_players[i].SetLevelPointer(&m_level);//HACK only temporary, we'll figure something out to properly separate layer's (server and client)
		players.emplace(i, &m_players[i]);
	}
	
	m_physicsEngine.Init(m_level, &players);

	m_exit = false;
	sf::Clock clock;


	// main loop
	while (!m_exit)
	{
		processEvents();

		float dt = clock.getElapsedTime().asSeconds();
		update(dt);
		clock.restart();

		draw();
	}
}


void Game::draw()
{
	m_window.clear();

	m_window.draw(m_levelView);

	for (unsigned i = 0; i < m_players.size(); ++i)
	{
		m_window.draw(m_players[i]);
	}

	m_window.display();
}


void Game::update(float deltaTime)
{
	m_physicsEngine.Update(deltaTime);
	for (unsigned i = 0; i < m_players.size(); ++i)
	{
		m_players[i].Update(deltaTime);
		m_players[i].CheckIsPlayerInBombRay(nullptr);
	}
	//It should be given a ptr to std::vector filled with bombRays
	//until only singlplayer is available, there is no need of changing collisions system
	//it will be completely rewritten anyway
}


void Game::processEvents()
{
	if (m_exit)
		return; // if exiting: skip event processing!

	sf::Event event;

	int x1 = 0;
	int y1 = 0;

	// HACK 1st iteration only, add class Input later
	// handle horizontal axis
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		x1 = -1;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		x1 = 1;

	// handle vertical axis
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		y1 = -1;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
     {
		y1 = 1;
     }



	int x2 = 0;
	int y2 = 0;

	// HACK 1st iteration only, add class Input later
	// handle horizontal axis
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		x2 = -1;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		x2 = 1;
    }
	// handle vertical axis
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		y2 = -1;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		y2 = 1;
    }

	// I have no idea how to make it in loop /Conrad
	m_players[0].OnMoveKeyPressed(x2, y2);
	m_players[1].OnMoveKeyPressed(x1, y1);

	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_exit = true;
			break;
		}
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
			m_players[0].OnActionKeyPressed();
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::LControl)
			m_players[1].OnActionKeyPressed();

		// handle more events
	}
}
