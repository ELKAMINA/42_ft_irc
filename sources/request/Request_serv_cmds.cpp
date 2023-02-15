/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request_serv_cmds.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcervoni <jcervoni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/08 18:20:59 by jcervoni          #+#    #+#             */
/*   Updated: 2023/02/13 18:37:37 by jcervoni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"


int Request::_pass(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	// std::cout << "siiiize " << serv->get_pass() << std::endl;
	// std::cout << entries[1] << entries[1].size() << std::endl;
	if((entries.size() + 1) > 2 || (entries.size() + 1) < 2)
	{
		req_validity = notEnough_params;
		return 1;
	}
	else if(cli->getNickName() != "UNDEFINED")
	{
		req_validity = already_registered;
		return 1;
	}
	else if ((entries.size() + 1) == 2)
	{
		// entries[0].resize(entries[0].size() - 1); // take off the \n
		if (entries[0] == serv->get_pass())
		{
			// std::cout << "PASS " << entries[0] << std::endl;
			req_validity = valid_req; // A changer 
			cli->setPwd(serv->get_pass());
			return 1;
		}
		else
		{
			req_validity = incorrect_pwd;
			return 1;
		}
	}
	return 0;
}

int Request::_nick(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	// std::cout << cli->getPwd() <<  cli->getUserName() << std::endl;
	// std::cout << "entry " << entries[0] << " size " << entries[0].size() << std::endl;
	// entries[0].resize(entries[0].size() - 1);
	if (entries.size() > 1 || entries.size() < 1)
	{
		req_validity = notEnough_params;
		return 1;	
	}
	else if (cli->getPwd() == "UNDEFINED" && cli->getUserName() == "UNDEFINED" )
	{
		req_validity = omitted_cmd;
		return 1;
	}
	else if (user_existence(entries[0], serv) == 0)
	{
		req_validity = nickname_exists;
		return 1;
	}
	else if (wrong_nickname() == 0)
	{
		req_validity = erroneous_nickname;
		return 1;
	}
	// std::cout << "hereeee " << entries[0] << entries[0].size() << std::endl;
	cli->setNickname(entries[0]);
	// _nickname_cli = entries[0];
	// std::cout << " OK c'est good " << std::endl
	return 0;
}

int Request::_user(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	// std::cout << cli->getPwd() <<  cli->getUserName() << std::endl;
	if (entries.size() < 4 || entries.size() > 4)
	{
		req_validity = notEnough_params;
		return 1;	
	}
	else if (cli->getPwd() == "UNDEFINED" && cli->getNickName() == "UNDEFINED" )
	{
		req_validity = omitted_cmd;
		return 1;
	}
	else
	{
		// std::cout << "hey   " << std::endl;
		// entries[0].resize(entries[0].size() - 1);
		int mde = atoi(entries[1].c_str());
		cli->setUsername(entries[0]);
		cli->setMode(mde, false); /* false ajouté par amina*/
		// if (entries[3][0])
			// entries[3].resize(entries[3].size() - 1);
		cli->setRealname(entries[3]);
		req_validity = welcome_msg;
		// std::cout << " OK c'est good " << std::endl;
	}
	return 0;
}


int Request::_privmsg(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	if(entries.size() < 2)
	{
		req_validity = notEnough_params;
		return 1;
	}
	else if (entries.size() >= 2)
	{
		if	(entries[0][0] != '&' && entries[0][0] != '#')
		{
			std::vector<std::string>::iterator it = entries.begin();
			std::string dest;
			dest =  entries[0];
			std::string message;
			entries.erase(it);
			if (_find(dest, serv) != serv->_req_per_id.end()->first)
			{
				if ((_find(dest, serv))->checkMode('a') == 1)
					message = (_find(dest, serv))->getAwayMessage();
				else if ((_find(dest, serv))->checkMode('i') == 1)
					req_validity = invisible_man;
				else
				{

					if (entries.size() >= 1)
					{
						size_t i = 0;
						while (i < entries.size())
						{
							message.append(entries[i]);
							message.append(" ");
							i++;
						}
					}
				}
				std::ostringstream oss;
				oss << ":" << cli->getNickName() << "!" << cli->getNickName() << "@" << cli->getRealName() << " PRIVMSG " << cli->getNickName() << " " << message << "\n";
				std::string var = oss.str();
				if (send(_find(dest, serv)->getFdClient(), var.c_str(), var.length(), 0) == -1)
					return (-1);
			}
			return 0;
		}
		if (entries[0][0] == '&' || entries[0][0] == '#')
		{
			Channel *tmp = existing_chan(&entries[0][1], serv);
			if (!tmp)
			{
				reply = errNoSuchChannel(cli->getNickName(), entries[0]);
				serv->_test = true;
			}
			else
			{
				if (message == "")
				{
					message.clear();
					size_t i = jo_nb_chan;
					while(i < entries.size())
					{
						message += entries[i];
						message += ' ';
						i++;
					}
				}
				tmp->cmd_lexer(*this);
			}
			serv->_chan_requests(this);
			return 2;
		}
	}
	return 5;
}

int Request::_notice(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	if(entries.size() < 2)
	{
		req_validity = notEnough_params;
		return 1;
	}
	else if (entries.size() >= 2)
	{
		if	(entries[0][0] != '&' && entries[0][0] != '#')
		{
			std::vector<std::string>::iterator it = entries.begin();
			std::string dest;
			dest =  entries[0];
			std::string message;
			entries.erase(it);
			if (_find(dest, serv) != serv->_req_per_id.end()->first)
			{
				if ((_find(dest, serv))->checkMode('a') == 1)
					return 0;
				else if ((_find(dest, serv))->checkMode('i') == 1)
					return 0;
				else
				{

					if (entries.size() >= 1)
					{
						size_t i = 0;
						while (i < entries.size())
						{
							message.append(entries[i]);
							message.append(" ");
							i++;
						}
					}
				}
				std::ostringstream oss;
				oss << ":" << cli->getNickName() << "!" << cli->getNickName() << "@" << cli->getRealName() << " NOTICE " << cli->getNickName() << " " << message << "\n";
				std::string var = oss.str();
				if (send(_find(dest, serv)->getFdClient(), var.c_str(), var.length(), 0) == -1)
					return (-1);
			}
			return 0;
		}
		if (entries[0][0] == '&' || entries[0][0] == '#')
		{
			Channel *tmp = existing_chan(&entries[0][1], serv);
			if (!tmp)
			{
				return 0;
			}
			else
			{
				if (message == "")
				{
					message.clear();
					size_t i = jo_nb_chan;
					while(i < entries.size())
					{
						message += entries[i];
						message += ' ';
						i++;
					}
				}
				tmp->cmd_lexer(*this);
			}
			serv->_chan_requests(this);
			return 2;
		}
	}
	return 5;
}

int	Request::_away(Client* cli, Server *serv)
{
	(void)serv;
	
	if (entries.size() == 0 || (entries.size() == 1 && entries[1] == ""))
	{
		if(cli->checkMode('a') == 1)
		{
			reply = rpl_unaway(cli->getNickName(), ":You are no longer marked as being away!\n");
			cli->setMode('a', false);
		}
	}
	else if (entries.size() == 2 && entries[1][0] == ':')
	{
		size_t i = 0;
		std::string away;
		while(i < entries.size())
		{
			away += entries[i];
			away += " ";
			i++;
		}
		cli->setAwayMessage(away);
	}
	else
		req_validity = invalid_req;
	return 0;
}

int Request::_list(Client* cli, Server* serv)
{

	(void)cli;
	if (_check_lists() != 0)
	{
		string rep = "";
		
		for (size_t i = 0; i < serv->_all_chanels.size(); i++){
			if (!serv->_all_chanels[i]->activeMode('s'))
				rep += "#" + serv->_all_chanels[i]->getName() + ", ";
		}
		if (rep.size() != 0)
		{
			rep.replace(rep.size() -2, 2, "\n");
		}
		this->reply = rep;
	}
	else
		reply = "Invalid request \n";
	return 0;
}

int Request::_cap(Client* cli, Server* serv)
{
	(void)cli;
	(void)serv;
	return 0;
}

int	Request::_names(Client* cli, Server *serv) /* For later - A revoiiiiiiiir */
{
	(void)cli;
	// beginning_with_diez(entries);
	// std::cout << "nb of channels " << jo_nb_chan << "entries.size() " << entries.size() << std::endl;
	if (_check_lists() != 0)
	{
		std::vector<std::string>::iterator it = entries.begin();
		while (it != entries.end())
		{
			std::cout << "iiiit " << (*it) << std::endl;
			it++;
		}
		if (reply == "UNDEFINED")
		{
			removing_sharp(entries);
			reply.clear();
			if (entries.size() == 0 && jo_nb_chan == 0)
			{
				// std::cout << "je rentre ici " << std::endl;
				chan_names(serv);
				noChan_names(serv);
			}
			else if (entries.size() >= 1)
			{
				// std::cout << "je rentre ici ? " << std::endl;
				size_t i = 0;
				while (i < entries.size() && jo_nb_chan != 0)
				{
					Channel* tmp = existing_chan(entries[i], serv);
					if (tmp)
					{
						if (tmp->activeMode('s') == false)
							tmp->cmd_lexer(*this);
						reply += rpl_endofnames(tmp->getName(), "option");
					}
					i++;
				}
			}
		}
		
	}
	serv->_chan_requests(this);
	return 0;
}

int	Request::_invite(Client* cli, Server *serv)
{
	if (entries.size() < 2)
		reply = errNeedMoreParams(cli->getNickName(), _command);
	else
	{
		Channel* tmp = existing_chan(&entries[0][1], serv);
		if (tmp)
			tmp->cmd_lexer(*this);
		else
			reply = errNoSuchChannel(cli->getNickName(), entries[0]);
	}
	serv->_chan_requests(this);
	return 0;
}

int Request::_wallops(Client* cli, Server *serv)
{
	if (entries.size() < 2)
		reply = errNeedMoreParams(cli->getNickName(), _command);
	else
	{
		std::vector<Client*>::iterator it = serv->_all_clients.begin();
		while (it != serv->_all_clients.end())
		{
			if ((*it)->checkMode('w') == true)
			{
				for(size_t i = 0; i < entries.size(); i++)
				{
					reply += entries[i];
					reply += " ";
				}
				serv->_chan_requests(this);
			}
			it++;
		}
	}
	return 0;
}

int Request::_kill(Client* cli, Server* serv)
{
	if (entries.size() < 2)
		reply = errNeedMoreParams(cli->getNickName(), _command);
	else if(cli->checkMode('o') == false)
		reply = errNoPrivileges(":Permission Denied= You're not an IRC operator\n", "opti");
	else
	{
		Client* tmp = _find(entries[0], serv);
		if(!tmp)
			reply = errNoSuchNick(entries[0], entries[0]);
		else
		{
			std::string message = " :You are getting killed by " + cli->getNickName();
			if	(send(tmp->getFdClient(), message.c_str(), message.length(), 0) == -1)
				perror("Big time for welcoming_ Bravo");
			/* Supprimer le user from Chan et de toutes leslistes dans lesquelles il existe !!! */
		}
	}
	return 0;
}

int	Request::_oper(Client* cli, Server *serv) /* For later */
{
	if (entries.size() != 2)
		reply = errNeedMoreParams(cli->getNickName(), _command);
	else if (entries.size() == 2)
	{
		if (serv->_opers[entries[0]] == entries[1])
		{
			reply = rpl_youreoper(":You are now an IRC operator\n", "op");
			cli->setMode('o', true);
		}
		else
			reply = errPasswMismatch(cli->getNickName(), ":Password incorrect\n");
	}
	else
		reply = errNoOperHost(":No O-lines for your host\n", "op");
	serv->_chan_requests(this);
	return 0;
}
